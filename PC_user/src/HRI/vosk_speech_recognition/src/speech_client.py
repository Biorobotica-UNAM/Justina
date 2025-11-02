#!/usr/bin/env python3
"""
Speech Recognition Client Node for ROS 2
----------------------------------------
This node acts as a client to request speech recognition
from the VoskSpeechRecognizer service.
"""

import rclpy
from rclpy.node import Node
from vosk_speech_recognition.srv import SpeechRecog


class SpeechRecognitionClient(Node):
    """Client node that connects to the Vosk speech recognition service."""

    def __init__(self, service_name: str = "speech_recognition"):
        super().__init__("speech_recognition_client")

        self.client = self.create_client(SpeechRecog, service_name)
        self.get_logger().info(f"Waiting for service '{service_name}'...")

        if not self.client.wait_for_service(timeout_sec=10.0):
            self.get_logger().error(f"Service '{service_name}' not available.")
            raise RuntimeError("Speech recognition service unavailable.")

        self.get_logger().info(
            f"Connected to speech recognition service '{service_name}'."
        )

    # ----------------------------------------------------------------
    # Speech Recognition
    # ----------------------------------------------------------------

    def recognize_voice(self, grammar: str = "") -> str:
        """
        Send a request to the speech recognition service.
        If a grammar file name is provided, it will be used for recognition.
        """
        request = SpeechRecog.Request()
        request.is_speech_recog_enabled = True
        request.grammar = grammar

        future = self.client.call_async(request)
        rclpy.spin_until_future_complete(self, future)

        if not future.result():
            self.get_logger().error("No response received from the service.")
            return ""

        response = future.result()

        if not response.success:
            self.get_logger().warning("Service responded but recognition failed.")
            return ""

        self.get_logger().info(f"Recognized text: {response.text_recog}")
        return response.text_recog

    # ----------------------------------------------------------------
    # Shutdown
    # ----------------------------------------------------------------

    def close(self):
        """Shut down the client node cleanly."""
        self.get_logger().info("Closing speech recognition client.")
        self.destroy_node()
