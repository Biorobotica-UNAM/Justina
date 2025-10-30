# vosk_speech_recognition/launch/speech_recognizer_launch.py

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    pkg_share = get_package_share_directory("vosk_speech_recognition")

    # Parámetros
    language = "en"
    model_path = os.path.join(
        pkg_share, "model"
    )  # tu modelo ya está en install/share/vosk_speech_recognition/model
    sample_rate = 44100
    speech_service = "/speech_recognition"
    grammar_path = os.path.join(pkg_share, "grammar")  # tu carpeta grammar

    return LaunchDescription(
        [
            Node(
                package="vosk_speech_recognition",
                executable="speech_recognizer.py",
                name="vosk_speech_recognizer",
                output="screen",
                parameters=[
                    {
                        "language": language,
                        "model_path": model_path,
                        "sample_rate": sample_rate,
                        "speech_service": speech_service,
                        "grammar_path": grammar_path,
                    }
                ],
            )
        ]
    )
