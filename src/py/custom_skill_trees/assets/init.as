var ICON_OFFSETS: Object = {
  CST_Roland_Icon: {
     Icon4: { x:    1, y:  116 },
     Icon5: { x:    7, y:  126 },
     Icon6: { x:  -17, y:  104 },
     Icon7: { x:   -5, y:  126 },
     Icon8: { x:    5, y:  123 },
     Icon9: { x:   22, y:  126 },
    Icon10: { x:    7, y:  140 },
    Icon11: { x:   13, y:  248 },
    Icon12: { x:  -10, y:  237 },
    Icon13: { x:  -10, y:  140 },
    Icon14: { x:    7, y:  140 },
    Icon15: { x:   10, y:  140 },
    Icon16: { x:    7, y:  114 },
    Icon17: { x:   10, y:  114 },
    Icon18: { x:  -10, y:  207 },
    Icon19: { x:  -10, y:  114 },
    Icon20: { x:    7, y:  114 },
    Icon21: { x:   10, y:  114 },
    Icon22: { x:    7, y:  166 },
    Icon23: { x:   -5, y:  166 },
    Icon24: { x:   35, y:  266 }
  },
  CST_Mordecai_Icon: {
     Icon4: { x:    1, y:  116 },
     Icon5: { x:    3, y:  126 },
     Icon6: { x:  -17, y:  104 },
     Icon7: { x:   -5, y:  126 },
     Icon8: { x:    5, y:  123 },
     Icon9: { x:   74, y:  126 },
    Icon10: { x:    7, y:  140 },
    Icon11: { x:   16, y:  236 },
    Icon12: { x:  -13, y:  140 },
    Icon13: { x:  -10, y:  140 },
    Icon14: { x:    7, y:  140 },
    Icon15: { x:   14, y:  230 },
    Icon16: { x:    7, y:  114 },
    Icon17: { x:   10, y:  114 },
    Icon18: { x:  -13, y:  114 },
    Icon19: { x:   -4, y:  212 },
    Icon20: { x:    7, y:  114 },
    Icon21: { x:   10, y:  120 },
    Icon22: { x:    7, y:  166 },
    Icon23: { x:   -5, y:  166 },
    Icon24: { x:   35, y:  266 }
  },
  CST_Lilith_Icon: {
     Icon4: { x:   13, y:  116 },
     Icon5: { x:    7, y:  126 },
     Icon6: { x:  -17, y:  112 },
     Icon7: { x:   -5, y:  126 },
     Icon8: { x:   12, y:  130 },
     Icon9: { x:  103, y:  126 },
    Icon10: { x:    7, y:  140 },
    Icon11: { x:   10, y:  140 },
    Icon12: { x:  -13, y:  140 },
    Icon13: { x:  -10, y:  140 },
    Icon14: { x:   11, y:  235 },
    Icon15: { x:   10, y:  140 },
    Icon16: { x:    7, y:  114 },
    Icon17: { x:   16, y:  210 },
    Icon18: { x:  -10, y:  207 },
    Icon19: { x:  -10, y:  114 },
    Icon20: { x:    7, y:  114 },
    Icon21: { x:   15, y:  207 },
    Icon22: { x:    7, y:  166 },
    Icon23: { x:   -1, y:  262 },
    Icon24: { x:   22, y:  166 }
  },
  CST_Brick_Icon: {
     Icon4: { x:    1, y:  139 },
     Icon5: { x:  -25, y:  126 },
     Icon6: { x:  -22, y:  104 },
     Icon7: { x:   -5, y:  126 },
     Icon8: { x:   20, y:  123 },
     Icon9: { x:   22, y:  126 },
    Icon10: { x:    7, y:  140 },
    Icon11: { x:   16, y:  235 },
    Icon12: { x:  -13, y:  140 },
    Icon13: { x:   -6, y:  235 },
    Icon14: { x:   11, y:  235 },
    Icon15: { x:   10, y:  140 },
    Icon16: { x:    7, y:  114 },
    Icon17: { x:   10, y:  114 },
    Icon18: { x:  -13, y:  114 },
    Icon19: { x:  -10, y:  114 },
    Icon20: { x:    7, y:  114 },
    Icon21: { x:   10, y:  114 },
    Icon22: { x:    7, y:  166 },
    Icon23: { x:   -5, y:  166 },
    Icon24: { x:   26, y:  262 }
  }
};

var ICON_PREFIXES: Object = {
  R: "CST_Roland_Icon",
  M: "CST_Mordecai_Icon",
  L: "CST_Lilith_Icon",
  B: "CST_Brick_Icon",
  CR: "CST_CUSTOM_REGULAR_ICON", // Not implemented yet.
  CK: "CST_CUSTOM_KILL_ICON" // Not implemented yet.
};

var KILL_SKILL_ICONS: Object = {
    CST_Roland_Icon:   [11,12,18,24],
    CST_Mordecai_Icon: [11,15,19,24],
    CST_Lilith_Icon:   [14,17,18,21,23],
    CST_Brick_Icon:    [11,13,14,24]
};

function is_kill_skill_icon(prefix: String, number: Number) {
  if (prefix == "CST_CUSTOM_KILL_ICON") {
    return true;
  } else if (prefix == "CST_CUSTOM_REGULAR_ICON") {
    return false;
  }

  var kill_list: Array = KILL_SKILL_ICONS[prefix];
  for (var i = 0; i < kill_list.length; i++) {
      if (kill_list[i] == number) {
          return true;
      }
  }
  return false;
}

function icon_offset(prefix: String, index: Number) {
  var perChar = ICON_OFFSETS[prefix];
  if (perChar != undefined) {
      return perChar["Icon" + index];
  }
  return undefined;
}

function create_skill(type: String, id: Number, x: Number, y: Number) {
  var icon_prefix = ICON_PREFIXES[type];
  var is_kill_skill = is_kill_skill_icon(icon_prefix, id);
  var suffix = is_kill_skill ? "KillSkill" : "Regular"

  var sel = skills.attachMovie("CST_Select_" + suffix, "sel" + id, 800 + id);
  if (is_kill_skill) {
    sel._x = (x / 20.0);
    sel._y = (y / 20.0);
  } else {
    sel._x = ((x - 93) / 20.0);
    sel._y = ((y - 92) / 20.0);
  }

  var cell = skills.attachMovie("CST_Frame_" + suffix, "cell" + id, 900 + id);
  cell._x = (x / 20.0);
  cell._y = (y / 20.0);

  var icon = skills.attachMovie(icon_prefix + id, "icon" + id, 1000 + id);
  var offset = icon_offset(icon_prefix, id);
  icon._x = (x + offset.x) / 20.0;
  icon._y = (y + offset.y) / 20.0;
};

var SKILL_TREE_OFFSETS: Array = [
  { x: 0000, y: 0000 },
  { x: 0000, y: 0000 },
  { x: 3992, y: 1237 },
  { x: 0000, y: 0000 },
  { x:  532, y: 3409 },
  { x: 2269, y: 3409 },
  { x: 4132, y: 3409 },
  { x: 5880, y: 3409 },
  { x: 7692, y: 3409 },
  { x: 9429, y: 3409 },
  { x:  532, y: 5035 },
  { x: 2269, y: 5035 },
  { x: 4132, y: 5035 },
  { x: 5869, y: 5035 },
  { x: 7692, y: 5035 },
  { x: 9429, y: 5035 },
  { x:  532, y: 6661 },
  { x: 2269, y: 6661 },
  { x: 4132, y: 6661 },
  { x: 5869, y: 6661 },
  { x: 7692, y: 6661 },
  { x: 9429, y: 6661 },
  { x: 1392, y: 8289 },
  { x: 4984, y: 8289 },
  { x: 8577, y: 8289 }
];

function create_standard_skill_tree(character: String) {

  // action skill
  var icon_prefix = ICON_PREFIXES[character];
  var action_skill = skills.attachMovie(icon_prefix + "2", "icon2", 308);
  action_skill._x = (3992 / 20.0);
  action_skill._y = (1237 / 20.0);

  // actual skills
  for (var i = 4; i < SKILL_TREE_OFFSETS.length; ++i) {
    var offset = SKILL_TREE_OFFSETS[i];
    create_skill(character, i, offset.x, offset.y);
  }

  // ensure this appears on top of the skill tree
  var artifact_selection = skills.attachMovie("CST_Artifact_Selection", "comm", 1250);
  artifact_selection._x = (-65) / 20.0;
  artifact_selection._y = (-1253) / 20.0;
  artifact_selection.visible = false;
}

function create_skill_tree_from_str(str: String) {
  // action skill
  var icon_prefix = ICON_PREFIXES["R"];
  var action_skill = skills.attachMovie(icon_prefix + "2", "icon2", 308);
  action_skill._x = (3992 / 20.0);
  action_skill._y = (1237 / 20.0);

  for (var i = 0; i < str.length; ++i) {
    var elem = str.charAt(i);
    var offset = SKILL_TREE_OFFSETS[i+4];
    create_skill(elem, i+4, offset.x, offset.y);
  }

  // ensure this appears on top of the skill tree
  var artifact_selection = skills.attachMovie("CST_Artifact_Selection", "comm", 1250);
  artifact_selection._x = (-65) / 20.0;
  artifact_selection._y = (-1253) / 20.0;
  artifact_selection.visible = false;
}

stop();
header.gotoAndStop("skills");
Key.removeListener(logListener);
Key.removeListener(logWindowListener);
Key.removeListener(characterListener);
Key.removeListener(inventoryListener);
Key.removeListener(mapListener);
Key.removeListener(inventoryListListener);
Key.removeListener(inventoryCellListener);
Key.removeListener(movieCloseListener);
currentScreen = "skills";
topLevel_mc = skills;
flash.external.ExternalInterface.call("extSetCurrentScreen","skills");