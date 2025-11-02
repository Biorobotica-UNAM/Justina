#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from std_msgs.msg import String, Bool
import subprocess

speech_topic = "voice/speech"
speech_status_topic = "voice/speech_status"
queue_size = 50

class SpeechSpeaker(Node):
    def __init__(self) -> None:
        super().__init__("speech_speaker")

        # Parámetros configurables
        self.declare_parameter("enable_tts", True)
        self.declare_parameter("tts_voice", "en-us+f2")  # Voz de mujer
        self.declare_parameter("tts_speed", 190)          # Velocidad de voz

        # Cargar parámetros
        self._enable: bool = bool(self.get_parameter("enable_tts").value)
        self._voice: str = str(self.get_parameter("tts_voice").value)
        self._speed: int = int(self.get_parameter("tts_speed").value)

        self._speech_sub = self.create_subscription(String, speech_topic, self._on_message, queue_size)
        self._status_pub = self.create_publisher(Bool, speech_status_topic, queue_size)

        self.get_logger().info(f"SpeechSpeaker started. Voice: {self._voice}, Speed: {self._speed}")

    def _on_message(self, msg: String) -> None:
        text = msg.data.strip()
        if not text:
            self.get_logger().warn("Received empty text, ignoring")
            return

        self.get_logger().info(f"Speaking: '{text}'")
        if self._enable:
            self._speak(text)

        # Publicar que terminó de hablar
        self._status_pub.publish(Bool(data=True))

    def _speak(self, text: str) -> None:
        try:
            subprocess.run([
                "espeak-ng",
                "-v", self._voice,
                "-s", str(self._speed),
                text
            ], check=True)
        except FileNotFoundError:
            self.get_logger().error("espeak-ng not found. Install it to use TTS.")
        except subprocess.CalledProcessError as exc:
            self.get_logger().error(f"Error executing espeak-ng: {exc}")


def main(args=None) -> None:
    rclpy.init(args=args)
    node = SpeechSpeaker()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        node.get_logger().info("SpeechSpeaker stopped by keyboard.")
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()

