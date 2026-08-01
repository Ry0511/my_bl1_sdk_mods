from pathlib import Path
from mods_base import SETTINGS_DIR
from unrealsdk import logging

from .default_skills import (
    ALL_ROLAND_SKILLS,
    INDEX_MAPPING_ROW_MAJOR,
    create_skill_look_up_table,
)

PRESET_DIR = SETTINGS_DIR / "custom_skill_trees" / "presets"
PRESET_DIR.mkdir(parents=True, exist_ok=True)


class PresetFile:
    file: Path
    skills: list[str]

    def __init__(self, file: Path):
        self.file = file
        self.skills = list()

        lut = {v.lower(): k for k, v in create_skill_look_up_table().items()}
        error_count = 0

        for line in map(lambda s: s.strip().lower(), file.read_text().splitlines()):
            if (
                line is None  # pyright: ignore[reportUnnecessaryComparison]
                or len(line) == 0
                or line[0] in "#;"
            ):
                continue

            if (skill := lut.get(line, None)) is not None:
                if line in self.skills:
                    error_count += 1
                    logging.error(
                        f"'{line}' already exists in the skill tree."
                        + f" duplicate skills are not supported"
                    )
                self.skills.append(skill)
            else:
                error_count += 1
                logging.error(f"'{line}' does not map to any known skill")

        if error_count > 0:
            raise ValueError("invalid preset file selected")
        elif len(self.skills) != len(ALL_ROLAND_SKILLS):
            raise ValueError(
                f"expecting {len(ALL_ROLAND_SKILLS)} skills but got {len(self.skills)}"
            )

    @staticmethod
    def create_example_file() -> None:
        file = PRESET_DIR / "example.txt"
        content = "; this is a comment\n# this is also a comment\n\n; blank lines are ignored\n\n"
        content += (
            "; 7x3 groups, one for each tree left, middle, and right. "
            + "The following creates Kevin Hearts tree\n"
        )
        lut = create_skill_look_up_table()
        index = 0
        for i in INDEX_MAPPING_ROW_MAJOR:
            content += f"{lut[ALL_ROLAND_SKILLS[i]]}\n"
            index += 1
            if index > 0 and (index % 7) == 0:
                content += "\n"
        _ = file.write_text(content, encoding="utf-8")


PresetFile.create_example_file()
