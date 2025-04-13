from ui_utils import *

buttons: list[OptionBoxButton] = [
    OptionBoxButton(name="Button A", tip="Tip for A"),
    OptionBoxButton(name="Button B", tip="Tip for B"),
    OptionBoxButton(name="Button C", tip="Tip for C"),
    OptionBoxButton(name="Button D", tip="Tip for D"),
]

box: ReorderBox = ReorderBox(
    title="This is my title",
    message="This is my message",
    tooltip="This is my tooltip",
    buttons=buttons
)

def _on_cancel(self: ReorderBox) -> None:
    print(f"[REORDER_BOX] ~ Cancelled '{self.title}'")

def _on_select(self: ReorderBox, btn: OptionBoxButton):
    print(f"[REORDER_BOX] ~ Selected '{self.title}', '{btn.name}'")

def _on_move(self: ReorderBox, btn: OptionBoxButton):
    print(f"[REORDER_BOX] ~ Moving '{self.title}', '{btn.name}'")


box.on_select = _on_select
box.on_cancel = _on_cancel
box.on_move = _on_move

box.show()
