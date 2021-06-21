JAVA="$1"
CLASS="$2"
JAVA_HOME="$3"

OUT="out/$CLASS"
mkdir -p "$OUT" || exit 42

R_OUT="$OUT/reference_stdout"
R_ERR="$OUT/reference_stderr"
R_STATUS="$OUT/reference_status"

S_OUT="$OUT/schoko_stdout"
S_ERR="$OUT/schoko_stderr"
S_STATUS="$OUT/schoko_status"

"$JAVA" -Djava.library.path="$PWD" -classpath tests.jar "$CLASS" $4 1>"$R_OUT" 2>"$R_ERR"
echo "$?" > "$R_STATUS"

./SchokoVM --java-home $JAVA_HOME -classpath tests.jar "$CLASS" $4 1>"$S_OUT" 2>"$S_ERR"
# "$JAVA" -XXaltjvm="$PWD" -Xbootclasspath:../jdk/exploded-modules -Xjavahome:$JAVA_HOME -classpath tests.jar "$CLASS" 1>"$S_OUT" 2>"$S_ERR"
echo "$?" > "$S_STATUS"

X=0
echo "============================== status =============================="
diff --side-by-side --text "$R_STATUS" "$S_STATUS" || X=1
echo "============================== stdout =============================="
diff --side-by-side --text "$R_OUT" "$S_OUT"       || X=1
echo "============================== stderr =============================="
diff                --text "$R_ERR" "$S_ERR"       || X=1
echo "===================================================================="
exit "$X"
