#include <cctype>
#include <cmath>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./tb.h"

bool isDelim(char inChar) {
  return strchr(" ;,+-<>/*%^=()", inChar) != 0 || inChar == '\t' ||
         inChar == '\r' || inChar == '\n' || inChar == '\0';
}

bool isWhite(char inChar) { return inChar == ' ' || inChar == '\t'; }

void error(Error inErrorID) {
  if (gErrDict.find(inErrorID) == gErrDict.end()) {
    fprintf(stderr, "SYSTEM ERROR (no such error ID).\n");
    exit(-2);
  }
  printf("%s\n", gErrDict[inErrorID]);
  exit(-1);
}

void fpush(LoopInfo inLoopInfo) { gFStack.push_back(inLoopInfo); }
LoopInfo fpop() {
  if (gFStack.size() == 0) {
    error(MismatchNext);
  }
  LoopInfo loopInfo = gFStack.back();
  gFStack.pop_back();
  return loopInfo;
}

void gpush(char *inProgPos) { gGStack.push_back(inProgPos); }
char *gpop() {
  if (gGStack.size() == 0) {
    error(MismatchReturn);
  }
  char *progPos = gGStack.back();
  gGStack.pop_back();
  return progPos;
}

Keyword lookUpCommand(char *inToken) {
  return gMap.find(inToken) == gMap.end() ? INVALID : gMap[inToken];
}

bool isUniqueLabel(char *inLabel) {
  return gLabelTable.find(inLabel) == gLabelTable.end();
}
char *findLabel(char *inLabel) {
  return gLabelTable.find(inLabel) == gLabelTable.end() ? nullptr
                                                        : gLabelTable[inLabel];
}

char *loadProgram(char *inFileName) {
  FILE *fp = fopen(inFileName, "rb");
  if (fp == nullptr) {
    return nullptr;
  }
  char *bufTop = (char *)malloc(ProgSize);
  if (bufTop != nullptr) {
    char *p = bufTop;
    for (int i = 0; feof(fp) == 0 && i < ProgSize; p++, i++) {
      *p = getc(fp);
    }
    *(p - 2) = '\0'; // プログラムのヌル終端
                     // (-2は最後のp++で進んだ1と、配列の添え字の分の1)
  }
  fclose(fp);
  return bufTop;
}

Type getToken() {
  gTokenType = Invalid;
  gTokVal = INVALID;
  char *dest = gToken;

  if (*gProgPos == '\0') {
    // ファイル終端
    *gToken = '\0';
    gTokVal = FINISHED;
    gTokenType = Delimiter;
    return gTokenType;
  }

  while (isWhite(*gProgPos)) {
    ++gProgPos;
  }

  if (gProgPos[0] == '\r' && gProgPos[1] == '\n') {
    // crlf
    gProgPos++;
    gToken[0] = '\r';
    gToken[1] = '\n';
    gToken[2] = '\0';
    gTokVal = EOL;
    gTokenType = Delimiter;
    return gTokenType;
  }

  if (*gProgPos == '\n') {
    // lf
    gProgPos++;
    gToken[0] = '\n';
    gToken[1] = '\0';
    gTokVal = EOL;
    gTokenType = Delimiter;
    return gTokenType;
  }

  if (strchr("+-*^/%=;(),>?", *gProgPos)) {
    *dest = *gProgPos;
    gProgPos++;
    *(++dest) = '\0';
    gTokenType = Delimiter;
    return gTokenType;
  }

  if (*gProgPos == '"') {
    // クウォートで囲まれた文字列
    gProgPos++;
    while (*gProgPos != '"' && *gProgPos != '\r') {
      *dest++ = *gProgPos++;
    }
    if (*gProgPos == '\r') {
      error(Parentheses);
    }
    gProgPos++;
    *dest = '\0';
    gTokenType = Quote;
    return gTokenType;
  }

  if (isdigit(*gProgPos)) {
    // 数値
    while (isDelim(*gProgPos) == false) {
      *dest++ = *gProgPos++;
    }
    *dest = '\0';
    gTokenType = Number;
    return gTokenType;
  }

  if (isalpha(*gProgPos)) {
    // 変数かコマンド
    while (isDelim(*gProgPos) == false) {
      *dest++ = *gProgPos++;
    }
    *dest = '\0';
    gTokenType = Number;
    return gTokenType;
  }

  *dest = '\0';

  if (gTokenType == String) {
    // コマンドか変数か確認
    gTokVal = lookUpCommand(gToken);
    gTokenType = gTokVal == INVALID ? Variable : Command;
  }

  return gTokenType;
}

void putBack() {
  for (char *t = gToken; *t != '\0'; t++) {
    gProgPos--;
  }
}

int findVar(char *inToken) {
  if (isalpha(*inToken) == 0) {
    error(NotAVariable);
  }
  return gVariables[toupper(*gToken) - 'A'];
}

int primitive() {
  int ret = -1;
  switch (gTokenType) {
  case Variable:
    ret = findVar(gToken);
    getToken();
    break;
  case Number:
    ret = atoi(gToken);
    getToken();
    break;
  default:
    error(SyntaxError);
  }
  return ret;
}

int unary(char inOp, int inX) { return inOp == '-' ? -inX : inX; }

int arith(char inOp, int inRight, int inLeft) {
  int ret = -1;
  switch (inOp) {
  case '-':
    ret = inRight - inLeft;
    break;
  case '+':
    ret = inRight + inLeft;
    break;
  case '*':
    ret = inRight * inLeft;
    break;
  case '/':
    ret = inRight / inLeft;
    break;
  case '%':
    ret = inRight % inLeft;
    break;
  case '^':
    ret = (int)pow(inRight, inLeft);
    break;
  }
  return ret;
}

void level6(int *outResult) {
  if (*gToken == '(' && gTokenType == Delimiter) {
    getToken();
    level2(outResult);
    if (*gToken != ')') {
      error(Parentheses);
    }
    getToken();
  } else {
    *outResult = primitive();
  }
}

void level5(int *outResult) {
  char op = 0;
  if (gTokenType == Delimiter && (*gToken == '+' || *gToken == '-')) {
    op = *gToken;
    getToken();
  }
  level6(outResult);
  if (op != 0) {
    *outResult = unary(op, *outResult);
  }
}

void level4(int *outResult) {
  level5(outResult);
  if (*gToken == '^') {
    getToken();
    int hold;
    level4(&hold);
    *outResult = arith('^', *outResult, hold);
  }
}

void level3(int *outResult) {
  level4(outResult);
  char op;
  while ((op = *gToken) == '*' || op == '/' || op == '%') {
    getToken();
    int hold;
    level4(&hold);
    *outResult = arith(op, *outResult, hold);
  }
}

void level2(int *outResult) {
  level3(outResult);
  char op;
  while ((op = *gToken) == '+' || op == '-') {
    getToken();
    int hold;
    level3(&hold);
    *outResult = arith(op, *outResult, hold);
  }
}

int getExp() {
  getToken();
  if (*gToken == '\0') {
    error(NoExp);
  }
  int result;
  level2(&result);
  putBack();
  return result;
}

void assignment() {
  getToken();
  if (isalpha(*gToken) == 0) {
    error(NotAVariable);
  }
  int var = toupper(*gToken) - 'A';
  getToken();
  if (*gToken != '=') {
    error(NoEqualSign);
  }
  gVariables[var] = getExp();
}

void findEol() {
  while (*gProgPos != '\n' && *gProgPos != '\0') {
    ++gProgPos;
  }
  if (*gProgPos) {
    gProgPos++;
  }
}

void scanLabels() {
  char *progPosBackup = gProgPos;
  getToken();
  if (gTokenType == Number) {
    gLabelTable[gToken] = gProgPos;
  }
  findEol();
  do {
    getToken();
    if (gTokenType == Number) {
      if (isUniqueLabel(gToken) == false) {
        error(DupLabel);
      }
      gLabelTable[gToken] = gProgPos;
    }
    if (gTokVal != EOL) {
      // 次の行を見ていく
      findEol();
    }
  } while (gTokVal != FINISHED);
  gProgPos = progPosBackup;
}

void execPrint() {
  char lastDelim;
  int len = 0;
  do {
    getToken(); // get next list item
    if (gTokVal == EOL || gTokVal == FINISHED) {
      break;
    }
    if (gTokenType == Quote) {
      // 文字列の時
      printf("%s", gToken);
      len += strlen(gToken);
      getToken();
    } else {
      // 文の時
      putBack();
      int answer = getExp();
      getToken();
      len += printf("%d", answer);
    }
    lastDelim = *gToken;
    if (*gToken == ';') {
      // タブ幅分のスペースを詰める
      for (int spaces = 8 - (len % 8); spaces > 0; spaces--) {
        printf(" ");
      }
    } else if (*gToken == ',') {
      // 何もしない
    } else if (gTokVal != EOL && gTokVal != FINISHED) {
      // そんなわけはない
      error(SyntaxError);
    }
  } while (*gToken == ';' || *gToken == ',');
  if (gTokVal == EOL || gTokVal == FINISHED) {
    if (lastDelim != ';' && lastDelim != ',') {
      printf("\n");
    }
  } else {
    error(SyntaxError);
  }
}

void execInput() {
  getToken(); // 入力メッセージの文字列がプロンプトにあるかチェック
  if (gTokenType == Quote) {
    // もしそうなら画面表示してコンマかチェック
    printf("%s", gToken);
    getToken();
    if (*gToken != ',') {
      error(Parentheses);
    }
    getToken();
  } else {
    // そうでなければ、?で入力を促す
    printf("? ");
  }
  char var = toupper(*gToken) - 'A'; // 入力用の変数を取得
  int i;
  scanf("%d", &i);
  gVariables[var] = i;
}

void execGoto() {
  getToken();
  char *pos = findLabel(gToken);
  if (pos == NULL) {
    error(NoSuchALabel);
  }
  gProgPos = pos;
}

void execGoSub() {
  getToken();
  char *pos = findLabel(gToken);
  if (pos == NULL) {
    error(NoSuchALabel);
  }
  gpush(gProgPos);
  gProgPos = pos;
}

void execReturn() { gProgPos = gpop(); }

void execIf() {
  int x = getExp(); // 左辺の文を取得
  getToken();       // オペレータを取得
  if (strchr("=<>", *gToken) == 0) {
    error(SyntaxError);
  }
  char op = *gToken;
  int y = getExp();
  int cond = 0;
  switch (op) {
  case '<':
    cond = x < y ? 1 : 0;
    break;
  case '>':
    cond = x > y ? 1 : 0;
    break;
  case '=':
    cond = x == y ? 1 : 0;
    break;
  }
  if (cond != 0) {
    // trueの時
    getToken();
    if (gTokVal != THEN) {
      error(NoThen);
    }
    // elseは次の行に書いてある
  } else {
    findEol();
  }
}

void execFor() {
  getToken(); // コントロール変数を読む
  if (isalpha(*gToken) == 0) {
    error(NotAVariable);
  }
  LoopInfo loopInfo;
  loopInfo.counterVarID = toupper(*gToken) - 'A';
  getToken(); // イコールのサインを読む
  if (*gToken != '=') {
    error(NoEqualSign);
  }
  int value = getExp();
  gVariables[loopInfo.counterVarID] = value; // 初期値を読み込む
  getToken();
  if (gTokVal != TO) {
    error(NoTo);
  }
  loopInfo.targetValue = getExp(); // ターゲットの値を読む
  if (value >= gVariables[loopInfo.counterVarID]) {
    // ループが一回でも実行可能な場合、スタックに情報を積む
    loopInfo.loopTopPos = gProgPos;
    fpush(loopInfo);
  } else {
    // そうではない場合、完全にスキップ
    while (gTokVal != NEXT) {
      getToken();
    }
  }
}

void execNext() {
  LoopInfo loopInfo = fpop(); // ループ情報を読み込む
  gVariables[loopInfo.counterVarID]++;
  if (gVariables[loopInfo.counterVarID] > loopInfo.targetValue) {
    return;
  }
  fpush(loopInfo);
  gProgPos = loopInfo.loopTopPos; // loop!
}

void execRem() {
  for (getToken(); gTokVal != EOL; getToken()) {
    // 何もしない
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: tinybasic <filename>\n");
    exit(-1);
  }
  gProgPos = loadProgram(argv[1]);
  if (gProgPos == NULL) {
    fprintf(stderr, "can not load program.\n");
    exit(-1);
  }
  scanLabels(); // プログラム中の最初のラベルを探す
  do {
    gTokenType = getToken();
    if (gTokenType == Variable) {
      putBack();
      assignment();
    } else {
      if (gTokVal == END) {
        break;
      }
      if (gCmdMap.find(gTokVal) != gCmdMap.end()) {
        gCmdMap[gTokVal]();
      }
    }
  } while (gTokVal != FINISHED);
  return 0;
}
