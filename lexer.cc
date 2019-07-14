/*
 * Copyright (C) Rida Bazzi, 2016
 *
 * Do not share this file with anyone
 */
#include <iostream>
#include <istream>
#include <vector>
#include <string>
#include <cctype>

#include "lexer.h"
#include "inputbuf.h"

using namespace std;

string reserved[] = { "END_OF_FILE",
    "IF", "WHILE", "DO", "THEN", "PRINT",
    "PLUS", "MINUS", "DIV", "MULT",
    "EQUAL", "COLON", "COMMA", "SEMICOLON",
    "LBRAC", "RBRAC", "LPAREN", "RPAREN",
    "NOTEQUAL", "GREATER", "LESS", "LTEQ", "GTEQ",
    "DOT", "NUM", "ID", "ERROR", "REALNUM", "BASE08NUM","BASE16NUM" // TODO: Add labels for new token types here (as string)
};

#define KEYWORDS_COUNT 5
string keyword[] = { "IF", "WHILE", "DO", "THEN", "PRINT" };

void Token::Print()
{
    cout << "{" << this->lexeme << " , "
         << reserved[(int) this->token_type] << " , "
         << this->line_no << "}\n";
}

LexicalAnalyzer::LexicalAnalyzer()
{
    this->line_no = 1;
    tmp.lexeme = "";
    tmp.line_no = 1;
    tmp.token_type = ERROR;
}

bool LexicalAnalyzer::SkipSpace()
{
    char c;
    bool space_encountered = false;

    input.GetChar(c);
    line_no += (c == '\n');

    while (!input.EndOfInput() && isspace(c)) {
        space_encountered = true;
        input.GetChar(c);
        line_no += (c == '\n');
    }

    if (!input.EndOfInput()) {
        input.UngetChar(c);
    }
    return space_encountered;
}

bool LexicalAnalyzer::IsKeyword(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return true;
        }
    }
    return false;
}

TokenType LexicalAnalyzer::FindKeywordIndex(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return (TokenType) (i + 1);
        }
    }
    return ERROR;
}

Token LexicalAnalyzer::ScanNumber()
{
  char c,c1,c2,c3;
  int flag = 0;
  char symbol_table[100];

  input.GetChar(c);

  if (isdigit(c))
  {
    if (c == '0')
    {
      tmp.lexeme = "0";
    }
    else
    {
      tmp.lexeme = "";
      // Recursively GetChar
      while ((!input.EndOfInput() && isdigit(c)))
      {
        tmp.lexeme += c;
        input.GetChar(c);
        if( c == '8' || c == '9')
        {
          flag = 1;
        }
      }
      if (!input.EndOfInput())
      {
        input.UngetChar(c);
      }
    }
      // TODO: You can check for REALNUM, BASE08NUM and BASE16NUM here!
      input.GetChar(c1);
      // Handling BASE08NUM and BASE16NUM
      if( c1 == 'x')
      {
        input.GetChar(c2);
        if(c2 == ' ')
        {
          input.UngetChar(c2);
        }
        // Handling BASE08NUM case
        if(c2 == '0')
        {
          input.GetChar(c3);
          if(c3 == '8' && flag == 0)
          {
            tmp.lexeme = tmp.lexeme+c1+c2+c3;
            tmp.token_type = BASE08NUM;
            tmp.line_no = line_no;
            return tmp;
          }
          else
          {
            // Let some other function handle it and UngetChar
            input.UngetChar(c3);
          }
          input.UngetChar(c2);  //Because can be REALNUM instead so UngetChar
        }
        // Handling BASE16NUM case
        else if(c2 == '1')
        {
         input.GetChar(c3);
         if(c3 == '6')
         {
          tmp.lexeme = tmp.lexeme +c1+c2+c3;
          tmp.token_type = BASE16NUM;
          tmp.line_no = line_no;
          return tmp;
         }
         else
         {
           // Not a BASE16NUM case so UngetChar
           input.UngetChar(c3);
         }
         // Could be REALNUM so UngetChar
         input.UngetChar(c2);
        }
        // Could be part of an ID as it is not BASE08NUM or BASE16NUM so UngetChar
        input.UngetChar(c1);
      }
      // Handling REALNUM case
      else if( c1 == '.')
      {
        string temp2;

        input.GetChar(c2);
        if(isdigit(c2))
        {
          while (!input.EndOfInput() && isdigit(c2))
          {
            temp2 += c2;
            input.GetChar(c2);
          }

          if (!input.EndOfInput())
          {
            input.UngetChar(c2);
          }
          input.UngetString(temp2);
          input.GetChar(c2);
          tmp.lexeme += '.';
          while (!input.EndOfInput() && (isdigit(c2)))
          {
             tmp.lexeme += c2;
             input.GetChar(c2);
          }
          if (!input.EndOfInput())
          {
             input.UngetChar(c2);
          }

          tmp.token_type = REALNUM;
          tmp.line_no = line_no;
          return tmp;
        }
        else
        {
          input.UngetChar(c2);
        }
        input.UngetChar(c1);
      }
      // Handling the hex part of BASE16NUM
      else if(c1 == 'A' ||
              c1 == 'B' ||
              c1 == 'C' ||
              c1 == 'D' ||
              c1 == 'E' ||
              c1 == 'F')
      {
        int count = 0;
        int count2 = 0;

        input.UngetChar(c1);
        input.GetChar(symbol_table[count]);
        while(isdigit(symbol_table[count]) ||
             (!input.EndOfInput() && symbol_table[count] == 'A' ||
                                     symbol_table[count] == 'B' ||
                                     symbol_table[count] == 'C' ||
                                     symbol_table[count] == 'D' ||
                                     symbol_table[count] == 'E' ||
                                     symbol_table[count] == 'F' ))
        {
          count++;
          count2++;
          input.GetChar(symbol_table[count]);
        }
        if(symbol_table[count] == 'x')
        {
          input.GetChar(symbol_table[count+1]);
          if(symbol_table[count+1] == '1')
          {
           input.GetChar(symbol_table[count+2]);
           if(symbol_table[count+2] == '6')
           {
             while(count > -3)
             {
               input.UngetChar(symbol_table[count + 2]);
               count--;
             }
             while(count2 >  -3)
             {
                 count2--;
                 input.GetChar(c);
                 tmp.lexeme += c;
             }
                tmp.token_type = BASE16NUM;
                tmp.line_no = line_no;
                return tmp;
           }
           else
           {
               input.UngetChar(symbol_table[count+2]);
               input.UngetChar(symbol_table[count+1]);
           }
        }
        else
        {
           input.UngetChar(symbol_table[count+1]);
        }
       }
       while(count > 0)
       {
        input.UngetChar(symbol_table[count]);
        count--;
       }
       input.UngetChar(c1);
      }
      else
      {
        input.UngetChar(c1);
      }

      tmp.token_type = NUM;
      tmp.line_no = line_no;
      return tmp;
    }
    else
    {
      if (!input.EndOfInput())
      {
          input.UngetChar(c);
      }
      tmp.lexeme = "";
      tmp.token_type = ERROR;
      tmp.line_no = line_no;
      return tmp;
    }
}

Token LexicalAnalyzer::ScanIdOrKeyword()
{
    char c;
    input.GetChar(c);

    if (isalpha(c)) {
        tmp.lexeme = "";
        while (!input.EndOfInput() && isalnum(c)) {
            tmp.lexeme += c;
            input.GetChar(c);
        }
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.line_no = line_no;
        if (IsKeyword(tmp.lexeme))
            tmp.token_type = FindKeywordIndex(tmp.lexeme);
        else
            tmp.token_type = ID;
    } else {
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
    }
    return tmp;
}

TokenType LexicalAnalyzer::UngetToken(Token tok)
{
    tokens.push_back(tok);;
    return tok.token_type;
}

Token LexicalAnalyzer::GetToken()
{
    char c;

    // if there are tokens that were previously
    // stored due to UngetToken(), pop a token and
    // return it without reading from input
    if (!tokens.empty()) {
        tmp = tokens.back();
        tokens.pop_back();
        return tmp;
    }

    SkipSpace();
    tmp.lexeme = "";
    tmp.line_no = line_no;
    input.GetChar(c);
    switch (c) {
        case '.':
            tmp.token_type = DOT;
            return tmp;
        case '+':
            tmp.token_type = PLUS;
            return tmp;
        case '-':
            tmp.token_type = MINUS;
            return tmp;
        case '/':
            tmp.token_type = DIV;
            return tmp;
        case '*':
            tmp.token_type = MULT;
            return tmp;
        case '=':
            tmp.token_type = EQUAL;
            return tmp;
        case ':':
            tmp.token_type = COLON;
            return tmp;
        case ',':
            tmp.token_type = COMMA;
            return tmp;
        case ';':
            tmp.token_type = SEMICOLON;
            return tmp;
        case '[':
            tmp.token_type = LBRAC;
            return tmp;
        case ']':
            tmp.token_type = RBRAC;
            return tmp;
        case '(':
            tmp.token_type = LPAREN;
            return tmp;
        case ')':
            tmp.token_type = RPAREN;
            return tmp;
        case '<':
            input.GetChar(c);
            if (c == '=') {
                tmp.token_type = LTEQ;
            } else if (c == '>') {
                tmp.token_type = NOTEQUAL;
            } else {
                if (!input.EndOfInput()) {
                    input.UngetChar(c);
                }
                tmp.token_type = LESS;
            }
            return tmp;
        case '>':
            input.GetChar(c);
            if (c == '=') {
                tmp.token_type = GTEQ;
            } else {
                if (!input.EndOfInput()) {
                    input.UngetChar(c);
                }
                tmp.token_type = GREATER;
            }
            return tmp;
        default:
            if (isdigit(c)) {
                input.UngetChar(c);
                return ScanNumber();
            } else if (isalpha(c)) {
                input.UngetChar(c);
                return ScanIdOrKeyword();
            } else if (input.EndOfInput())
                tmp.token_type = END_OF_FILE;
            else
                tmp.token_type = ERROR;

            return tmp;
    }
}

int main()
{
    LexicalAnalyzer lexer;
    Token token;

    token = lexer.GetToken();
    token.Print();
    while (token.token_type != END_OF_FILE)
    {
        token = lexer.GetToken();
        token.Print();
    }
}
