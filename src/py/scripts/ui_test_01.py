from ui_utils import *

print("TRAINING BOX TEST")

box = TrainingBox(title="My Training Box", message="My message goes here", min_duration=5.0)


def _on_exit(self: TrainingBox) -> None:
    print(f"Training Box '{self.title}' Closed!")


box.on_exit = _on_exit

box.show()
