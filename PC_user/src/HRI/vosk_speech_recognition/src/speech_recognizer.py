#!/usr/bin/env python3
"""
Vosk Speech Recognizer Node for ROS 2
------------------------------------
This node provides a speech recognition service using the Vosk engine.
It supports dynamic grammar loading and configurable audio input.
"""

import os
import json
import rclpy
from rclpy.node import Node
from ament_index_python.packages import get_package_share_directory
from vosk import Model, KaldiRecognizer
import pyaudio

from vosk_speech_recognition.srv import SpeechRecog


class VoskSpeechRecognizer(Node):
    """Speech recognition node using the Vosk library for ROS 2."""

    def __init__(self):
        super().__init__("vosk_speech_recognizer")

        # --- Parameters ---
        self.language = self.declare_parameter("language", "en").value
        self.model_path = self.declare_parameter("model_path", "model").value
        self.sample_rate = self.declare_parameter("sample_rate", 16000).value
        self.grammar_path = self.declare_parameter("grammar_path", "").value
        self.service_name = self.declare_parameter("speech_service", "speech_recognition").value
        self.input_device_index = self.declare_parameter("input_device_index", None).value

        # --- Initialize Model ---
        self.model = self._load_model(self.model_path)
        if not self.model:
            self.get_logger().error("Failed to load Vosk model.")
            rclpy.shutdown()
            return

        # --- Initialize Audio ---
        self.audio, self.stream = self._init_audio_stream(self.sample_rate)

        # --- Create Speech Service ---
        self.service = self.create_service(
            SpeechRecog, self.service_name, self._handle_speech_service
        )
        self.get_logger().info(
            f"Speech recognition service ready: '{self.service_name}'"
        )

    # ----------------------------------------------------------------
    # Model and Grammar Loading
    # ----------------------------------------------------------------

    def _load_model(self, relative_path: str):
        """Load the Vosk model from the package directory."""
        try:
            package_dir = get_package_share_directory("vosk_speech_recognition")
            model_full_path = os.path.join(package_dir, relative_path)
        except Exception as e:
            self.get_logger().error(f"Failed to resolve package directory: {e}")
            return None

        if not os.path.exists(model_full_path):
            self.get_logger().error(f"Model not found at: {model_full_path}")
            return None

        try:
            self.get_logger().info(f"Loading model from: {model_full_path}")
            return Model(model_full_path)
        except Exception as e:
            self.get_logger().error(f"Error while loading model: {e}")
            return None

    def _load_grammar(self, grammar_file: str):
        """Load a JSON grammar file if provided."""
        try:
            package_dir = get_package_share_directory("vosk_speech_recognition")
            full_path = os.path.join(package_dir, "grammar", grammar_file)
            if not os.path.isfile(full_path):
                self.get_logger().warning(f"Grammar file not found: {full_path}")
                return None

            with open(full_path, "r") as file:
                grammar = json.load(file)
            self.get_logger().info(f"Grammar loaded successfully: {full_path}")
            return grammar

        except Exception as e:
            self.get_logger().warning(f"Failed to load grammar: {e}")
            return None

    # ----------------------------------------------------------------
    # Audio Initialization
    # ----------------------------------------------------------------

    def _init_audio_stream(self, rate: int):
        """Initialize PyAudio stream for microphone input."""
        audio = pyaudio.PyAudio()
        stream = audio.open(
            format=pyaudio.paInt16,
            channels=1,
            rate=rate,
            input=True,
            frames_per_buffer=4096,
            input_device_index=self.input_device_index,
        )
        return audio, stream

    # ----------------------------------------------------------------
    # Speech Recognition
    # ----------------------------------------------------------------

    def _create_recognizer(self, grammar_rules=None):
        """Create a Kaldi recognizer with or without grammar rules."""
        if grammar_rules:
            recognizer = KaldiRecognizer(
                self.model, self.sample_rate, json.dumps(grammar_rules)
            )
            self.get_logger().info("Using dynamic grammar rules.")
        else:
            recognizer = KaldiRecognizer(self.model, self.sample_rate)
            self.get_logger().info("Using default recognition mode.")

        recognizer.SetWords(True)
        return recognizer

    def _recognize_speech(self, recognizer: KaldiRecognizer) -> str:
        """Perform real-time speech recognition until a valid result is obtained."""
        self.get_logger().info("Listening for speech...")
        while rclpy.ok():
            audio_data = self.stream.read(4096, exception_on_overflow=False)
            if recognizer.AcceptWaveform(audio_data):
                result = json.loads(recognizer.Result())
                recognized_text = result.get("text", "")
                if recognized_text:
                    confidences = [w.get("conf", 0) for w in result.get("result", [])]
                    if confidences and min(confidences) > 0.8:
                        self.get_logger().info(
                            f"Recognized (high confidence): {recognized_text}"
                        )
                        return recognized_text
                    self.get_logger().info(
                        f"Ignored (low confidence): {recognized_text}"
                    )
        return ""

    # ----------------------------------------------------------------
    # ROS 2 Service Callback
    # ----------------------------------------------------------------

    def _handle_speech_service(self, request, response):
        """Handle the speech recognition service request."""
        if request.is_speech_recog_enabled:
            grammar_rules = (
                self._load_grammar(request.grammar) if request.grammar else None
            )
            recognizer = self._create_recognizer(grammar_rules)
            response.text_recog = self._recognize_speech(recognizer)
            response.success = True
        else:
            self.get_logger().info("Speech recognition is disabled by request.")
            response.success = True
            response.text_recog = ""
        return response

    # ----------------------------------------------------------------
    # Shutdown
    # ----------------------------------------------------------------

    def shutdown(self):
        """Gracefully close audio resources."""
        self.get_logger().info("Shutting down audio stream...")
        self.stream.stop_stream()
        self.stream.close()
        self.audio.terminate()


# --------------------------------------------------------------------
# Entry Point
# --------------------------------------------------------------------


def main(args=None):
    rclpy.init(args=args)
    recognizer_node = VoskSpeechRecognizer()

    try:
        rclpy.spin(recognizer_node)
    except KeyboardInterrupt:
        recognizer_node.get_logger().info("Keyboard interrupt, shutting down node.")
    finally:
        recognizer_node.shutdown()
        recognizer_node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
