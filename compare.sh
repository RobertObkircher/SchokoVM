JAVA="$1"
CLASS="$2"
OUT="$3"

mkdir -p "$OUT" || exit 42

R_OUT="$OUT/reference_stdout"
R_ERR="$OUT/reference_stderr"
R_STATUS="$OUT/reference_status"

S_OUT="$OUT/schoko_stdout"
S_ERR="$OUT/schoko_stderr"
S_STATUS="$OUT/schoko_status"

"$JAVA" --class-path tests.jar "$CLASS" 1>"$R_OUT" 2>"$R_ERR"
echo "$?" > "$R_STATUS"

./SchokoVM --class-path tests.jar "$CLASS" 1>"$S_OUT" 2>"$S_ERR"
echo "$?" > "$S_STATUS"

X=0
echo "============================== status =============================="
diff --side-by-side "$R_STATUS" "$S_STATUS" || X=1
echo "============================== stdout =============================="
diff --side-by-side "$R_OUT" "$S_OUT"       || X=1
echo "============================== stderr =============================="
diff --side-by-side "$R_ERR" "$S_ERR"       || X=1
echo "===================================================================="
exit "$X"
