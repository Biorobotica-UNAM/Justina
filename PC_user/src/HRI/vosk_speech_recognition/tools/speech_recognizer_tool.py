#!/usr/bin/env python3
"""
Speech Recognition Tool
-----------------------
A reusable wrapper to access the Vosk speech recognition service from any
part of the ROS 2 project.

This class ensures that ROS is initialized only once and allows calling
speech recognition using a simple `recognize_voice(grammar)` interface.
"""

from typing import Optional
import rclpy
from vosk_speech_recognition.src.speech_client import SpeechRecognitionClient


class SpeechRecognitionTool:
    """
    A clean and reusable wrapper for performing voice recognition in ROS 2.
    """

    def __init__(self, service_name: str = "speech_recognition") -> None:
        """
        Initialize the speech recognition tool.

        If ROS 2 is not initialized, this tool will initialize it.
        A single client node is shared across all instances of this class.

        Args:
            service_name (str): The ROS 2 service name to connect to.
        """
        self._owns_rclpy = False
        self._client: Optional[SpeechRecognitionClient] = None

        # Initialize ROS 2 only if it is not already initialized
        if not rclpy.ok():
            rclpy.init()
            self._owns_rclpy = True

        # Initialize the client if not already created
        if not hasattr(SpeechRecognitionTool, "_shared_client"):
            SpeechRecognitionTool._shared_client = SpeechRecognitionClient(service_name)

        self._client = SpeechRecognitionTool._shared_client

    # ----------------------------------------------------------------
    # Public API
    # ----------------------------------------------------------------
    def recognize_voice(self, grammar: str = "") -> str:
        """
        Request speech recognition from the Vosk service.

        Args:
            grammar (str): Optional grammar file name to use for recognition.

        Returns:
            str: The recognized text from speech input.
        """
        return self._client.recognize_voice(grammar)

    def close(self) -> None:
        """
        Close the client and shutdown ROS 2 only if this tool initialized it.
        """
        if self._owns_rclpy:
            self._client.close()
            rclpy.shutdown()
