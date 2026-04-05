#include <map>
#include <string>
#include <vector>

//-----------------------------------------------------------------------------

// コンフィグ
enum Config {
  NumOfLabels = 100,   // ラベルの数
  LabelLen = 10,       // ラベルの長さ
  NumOfGosubNest = 25, // GoSubのネストの深さ
  ProgSize = 100000    // プログラムサイズ
};
// トークン種別
enum Type {
  Invalid,   // 無効
  Delimiter, // 区切り文字
  Variable,  // 変数
  Number,    // 数値
  Command,   // コマンド
  String,    // 文字列
  Quote,     // 文字列のクウォート
};
// キーワード
enum Keyword {
  INVALID,
  PRINT,
  INPUT,
  IF,
  THEN,
  GOTO,
  FOR,
  NEXT,
  TO,
  GOSUB,
  RETURN,
  END,
  REM,
  EOL,
  FINISHED
};
// エラー
enum Error {
  SyntaxError,    // シンタックスエラー
  Parentheses,    // かっこ
  NoExp,          // 文がない
  NoEqualSign,    // FOR文のイコールがない
  NotAVariable,   // 変数ではない
  DupLabel,       // ラベル重複
  NoSuchALabel,   // そんなラベルはない
  NoThen,         // THENがない
  NoTo,           // FOR文のTOがない
  MismatchNext,   // NEXTが合わない
  MismatchReturn, // RETURNが合わない
  SystemError     // システムエラー
};

//-----------------------------------------------------------------------------

// ループ情報
struct LoopInfo {
  int counterVarID;
  int targetValue;
  char *loopTopPos;
};

//-----------------------------------------------------------------------------

// 区切り文字か?
bool isDelim(char inChar);
// 空白か?
bool isWhite(char inChar);
// エラーメッセージを出力して終了する
void error(Error inErrorID);
// ループ情報をpush
void fpush(LoopInfo inLoopInfo);
// ループ情報をpop
LoopInfo fpop();
// コールスタックをpush
void gpush(char *inProgPos);
// コールスタックをpop
char *gpop();
// トークンからコマンドを取得
Keyword lookUpCommand(char *inToken);
// ユニーク(すでに同じものがある)ラベルか?
bool isUniqueLabel(char *inLabel);
// ラベルを取得
char *findLabel(char *inLabel);

// プログラムをロードする
char *loadProgram(char *inFileName);
// トークンを取得する
Type getToken();
// 入力ストリームへのトークンを返す?
void putBack();
// 変数を取得
int findVar(char *inToken);
// 数値か変数の値を取得する
int primitive();
// 単項のプラスかマイナス?
int unary(char inOp, int inX);
// 算術
int arith(char inOp, int inRight, int inLeft);
// レベル2(以下、演算子の優先度に従い段階を組んでいる)
void level2(int *outResult);
// 括弧文の処理
void level6(int *outResult);
// 単項の+か-
void level5(int *outResult);
// 整数の指数
void level4(int *outResult);
// 2要素の掛け算か割り算
void level3(int *outResult);
// 2項の足し算か引き算
void level2(int *outResult);
// 式を取得
int getExp();
// 割り当て
void assignment();
// EORを見つける
void findEol();
// 全てのラベルを走査する?
void scanLabels();
// Print文を実行する
void execPrint();
// シンプルな形のBASIC INPUT コマンドを実行する
void execInput();
// GOTO文を実行する
void execGoto();
// GOSUB文を実行する
void execGoSub();
// RETURN文を実行する
void execReturn();
// IF文を実行する
void execIf();
// FOR文を実行する
void execFor();
// NEXT文を実行する
void execNext();
// REM文を実行する
void execRem();

//-----------------------------------------------------------------------------

// 解析すべき式の場所を保持する
static char *gProgPos;
// 変数
static int gVariables[26] = {0};
// トークン
static char gToken[80];
// トークン種別
static Type gTokenType;
static Keyword gTokVal;
// FOR-NEXT文のためのループ情報
static std::vector<LoopInfo> gFStack = std::vector<LoopInfo>();
// GOSUB-RETURNのためのコールスタック
static std::vector<char *> gGStack = std::vector<char *>();
// ラベル
static std::map<std::string, char *> gLabelTable =
    std::map<std::string, char *>();
// エラーとエラーメッセージの対応
static std::map<Error, const char *> gErrDict = {
    {MismatchReturn, "RETURN without GOSUB"}, {NoTo, "TO expected"},
    {Parentheses, "unbalanced parentheses"},  {NoThen, "THEN expected"},
    {NoEqualSign, "equals sign expected"},    {DupLabel, "duplicate label"},
    {MismatchNext, "NEXT without FOR"},       {SyntaxError, "syntax error"},
    {NoSuchALabel, "undefined label"},        {NoExp, "no expression present"},
    {NotAVariable, "not a variable"},         {SystemError, "system error"},
};
// 文字列とトークンの対応
std::map<std::string, Keyword> gMap = {
    {"print", PRINT}, {"input", INPUT},   {"if", IF},     {"then", THEN},
    {"goto", GOTO},   {"for", FOR},       {"next", NEXT}, {"to", TO},
    {"gosub", GOSUB}, {"return", RETURN}, {"end", END},   {"rem", REM}};
// キーワードとコマンドの対応
std::map<Keyword, void (*)()> gCmdMap = {
    {PRINT, execPrint}, {INPUT, execInput}, {GOTO, execGoto},
    {IF, execIf},       {GOSUB, execGoSub}, {RETURN, execReturn},
    {FOR, execFor},     {NEXT, execNext},   {REM, execRem},
};
