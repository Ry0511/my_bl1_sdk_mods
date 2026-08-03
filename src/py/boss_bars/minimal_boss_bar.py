from __future__ import annotations
from typing import TYPE_CHECKING
from collections.abc import Sequence

from .boss_bar import EntityState

if TYPE_CHECKING:
    from BL1.Engine import Canvas

X_OFFSET = 80
Y_OFFSET = 48
BAR_WIDTH = 128
BAR_HEIGHT = 9


class MinimalBossBar:
    def draw(self, canvas: Canvas, entities: Sequence[EntityState]) -> None:
        x, y = X_OFFSET, Y_OFFSET

        text_height = canvas.Font.GetMaxCharHeight()
        tex = canvas.DefaultTexture

        for entity in entities:
            canvas.SetPos(x, y)
            canvas.SetDrawColor(255, 255, 255, 255)
            canvas.DrawText(f"{entity.name}", False, 1.0, 1.0)
            y += text_height

            canvas.SetPos(x, y)
            canvas.SetDrawColor(255, 0, 0, 255)
            canvas.DrawRect(BAR_WIDTH * entity.health.normalised(), BAR_HEIGHT, tex)

            if (shield := entity.shield) is not None:
                canvas.SetPos(x, y)
                canvas.SetDrawColor(0, 210, 210, 255)
                canvas.DrawRect(BAR_WIDTH * shield.normalised(), BAR_HEIGHT, tex)

            canvas.SetPos(x, y)
            canvas.SetDrawColor(200, 200, 200, 255)
            canvas.DrawBox(BAR_WIDTH, BAR_HEIGHT)
            y += text_height
