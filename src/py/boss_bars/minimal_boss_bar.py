from __future__ import annotations
from typing import TYPE_CHECKING
from collections.abc import Sequence

from .boss_bar import EntityState

if TYPE_CHECKING:
    from BL1.Engine import Texture, Canvas

X_OFFSET = 80
Y_OFFSET = 48
BAR_WIDTH = 128
BAR_WIDTH_FRACTION = 0.15

HEALTH_BAR_HEIGHT = 12
SHIELD_BAR_HEIGHT = 10

BG_BAR_COLOUR = (60, 60, 60, 255)
SHIELD_BAR_COLOUR = (0, 210, 210, 255)
HEALTH_BAR_COLOUR = (255, 40, 40, 255)


class MinimalBossBar:
    def draw_bar_filled(
        self,
        canvas: Canvas,
        x: float,
        y: float,
        w: float,
        h: float,
        color: tuple[int, int, int, int],
        theta: float,
        tex: Texture,
    ) -> None:
        canvas.SetPos(x, y)
        canvas.SetDrawColor(*BG_BAR_COLOUR)
        canvas.DrawRect(w, h, tex)

        canvas.SetPos(x, y)
        canvas.SetDrawColor(*color)
        canvas.DrawRect(w * theta, h, tex)

    def draw(self, canvas: Canvas, entities: Sequence[EntityState]) -> None:
        x, y = X_OFFSET, Y_OFFSET

        text_height = canvas.Font.GetMaxCharHeight()
        tex = canvas.DefaultTexture
        width: float = canvas.ClipX
        bar_width = max(width * BAR_WIDTH_FRACTION, BAR_WIDTH)

        for entity in entities:
            canvas.SetPos(x, y)
            canvas.SetDrawColor(255, 255, 255, 255)
            canvas.DrawText(entity.name, False, 1.0, 1.0)
            y += text_height

            if (shield := entity.shield) is not None:
                self.draw_bar_filled(
                    canvas,
                    x,
                    y,
                    bar_width,
                    SHIELD_BAR_HEIGHT,
                    SHIELD_BAR_COLOUR,
                    shield.normalised(),
                    tex,
                )
                y += SHIELD_BAR_HEIGHT

            self.draw_bar_filled(
                canvas,
                x,
                y,
                bar_width,
                HEALTH_BAR_HEIGHT,
                HEALTH_BAR_COLOUR,
                entity.health.normalised(),
                tex,
            )
            y += text_height + 2
