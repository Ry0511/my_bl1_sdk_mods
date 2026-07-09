charclass.html = true;
charclass.htmlText = "<font size=\'10\'>Custom Character One</font>";

tree1.html = true;
tree1.htmlText = "<font size=\'9\'>Custom Branch 1</font>";

tree2.html = true;
tree2.htmlText = "<font size=\'9\'>Custom Branch 2</font>";

tree3.html = true;
tree3.htmlText = "<font size=\'9\'>Custom Branch 3</font>";

//
// 04,05, 06,07, 08,09,
// 10,11, 12,13, 14,15,
// 16,17, 18,19, 20,21,
//  22,    23,    24,
//

function create_skill(type: String, id: int, x: Number, y: Number) {

    var sel_offset = 0;
    if (type == "KillSkill") {
        sel_offset = 90;
    }

    var sel = attachMovie("CST_Select_" + type, "sel" + id, 500 + id);
    sel._x = ((x+sel_offset) / 20.0);
    sel._y = (y / 20.0);

    var cell_offset_x = sel_offset;
    var cell_offset_y = 0;
    if (type == "Regular") {
        cell_offset_x = 93;
        cell_offset_y = 92;
    }

    var cell = attachMovie("CST_Frame_" + type, "cell" + id, 600 + id);
    cell._x = ((x+cell_offset_x) / 20.0);
    cell._y = ((y+cell_offset_y) / 20.0);

    var icon_offset_x = 100;
    var icon_offset_y = 218;
    if (type == "KillSkill") {
        icon_offset_x = 103;
        icon_offset_y = 232;
    }

    var icon = attachMovie("CST_Mordecai_Icon" + id, "icon" + id, 700 + id);
    icon._x = ((x+icon_offset_x) / 20.0);
    icon._y = ((y+icon_offset_y) / 20.0);
};

//
// 04,05, 06,07, 08,09,
// 10,11, 12,13, 14,15,
// 16,17, 18,19, 20,21,
//  22,    23,    24,
//

create_skill("Regular", 4 , 439 , 3317);
create_skill("Regular", 5 , 2176, 3317);
create_skill("Regular", 10, 439 , 4943);
create_skill("KillSkill", 11, 2176, 4943);
create_skill("Regular", 16, 439 , 6569);
create_skill("Regular", 17, 2176, 6569);
create_skill("Regular", 22, 1299, 8197);

create_skill("Regular", 6 , 4039, 3317);
create_skill("Regular", 7 , 5776, 3317);
create_skill("KillSkill", 12, 4039, 4943);
create_skill("Regular", 13, 5776, 4943);
create_skill("KillSkill", 18, 4039, 6569);
create_skill("Regular", 19, 5776, 6569);
create_skill("Regular", 23, 4891, 8197);

create_skill("Regular", 8 , 7599, 3317);
create_skill("Regular", 9 , 9284, 3317);
create_skill("Regular", 14, 7599, 4943);
create_skill("Regular", 15, 9284, 4943);
create_skill("Regular", 20, 7599, 6569);
create_skill("Regular", 21, 9284, 6569);
create_skill("KillSkill", 24, 8564, 8197);
