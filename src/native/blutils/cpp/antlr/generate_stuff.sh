BASE_DIR=$(dirname "$0")

# .\antlr-4.13.2-complete.jar

temp="$1"
antlr_jar="${temp:-$ANTLR_JAR}";

if [ -z "$antlr_jar" ]; then
    echo "antlr jar not provided; it is first arg or the environment variable ANTLR_JAR"
    exit 1
fi

out_dir="$BASE_DIR/gen$2"

# Create Lexer
echo "Generating lexer into '$out_dir'"
java -jar "$antlr_jar" -Dlanguage=Cpp "$BASE_DIR/ue3_text_obj_lexer.g4" -package "_blutils" -o "$out_dir"

# Create Parser
echo "Generating parser into '$out_dir'"
java -jar "$antlr_jar" -Dlanguage=Cpp "$BASE_DIR/ue3_text_obj_parser.g4" -package "_blutils" -o "$out_dir"