from ui_utils import *

print("OPTION BOX TEST")

buttons = [
    OptionBoxButton("Option A", tip="This is option A"),
    OptionBoxButton("Option B", tip="This is option B"),
    OptionBoxButton("Option C", tip="This is option C"),
    OptionBoxButton("Option D", tip="This is option D"),
    OptionBoxButton("Option E", tip="This is option E"),
    OptionBoxButton("Option F", tip="This is option F"),
]

box = OptionBox(
    title="My Option Box",
    message="My message goes here",
    tooltip="This is my tooltip",
    buttons=buttons
)


def _on_select(self: OptionBox, btn: OptionBoxButton) -> None:
    print(f"[{self.title}] ~ Button '{btn.name}' Selected")


def _on_cancel(self: OptionBox) -> None:
    print(f"Option Box '{self.title}' Closed!'")


box.on_select = _on_select
box.on_cancel = _on_cancel

box.show()