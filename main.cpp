#include <stdio.h>
#include <stdlib.h>
enum hash_entry_datatype {
  HASH_ENTRY_INVALID,
  HASH_ENTRY_INT,
  HASH_ENTRY_FLOAT,
  HASH_ENTRY_STRING,
  HASH_ENTRY_ARRAY,
  HASH_ENTRY_BOOL,
  HASH_ENTRY_DICTIONARY
};

enum AST_node_type {
  AST_NODE_INVALID,
  AST_NODE_KEY,
  AST_NODE_VALUE,
  AST_NODE_DATATYPE
};

static int SIZE = 499;

struct hash_entry{
  hash_entry_datatype Type;
  void * Data;
};

struct hash_table{
  hash_entry *Entries;
  int Length; 
};

struct array_type{
  hash_entry_datatype Type;
  int Length;
  void *Data;
};

struct character_stack{
  int NextStackIndex;
  int MaxSize;
  char* Stack;
};
struct string{
  char *Data;
  int Length;
};

struct AST_node {
  int NumChildren;
  char *Key;
  hash_entry_datatype Type;
  void *Value;
  AST_node *Children;
};

int Push(character_stack *Stack, char Character){
  if(Stack->NextStackIndex-1 >= Stack->MaxSize){
    return 0;
  }else{
    Stack->Stack[Stack->NextStackIndex] = Character;
    Stack->NextStackIndex++;
    return 1;
  }
}
char Peek(character_stack *Stack){
  char Result;
  if(Stack->NextStackIndex == 0){
    Result = 0;
  }else{
    Result = Stack->Stack[Stack->NextStackIndex-1];
  }
  return Result;

}
char Pop(character_stack *Stack){
  char Result = Peek(Stack);
  if(Result){
    Stack->NextStackIndex--;
  }
  return Result;
}

bool IsEmpty(character_stack *Stack){
  return Stack->NextStackIndex == 0;
}

int Hash (const char* word)
{
    unsigned int hash = 0;
    for (int i = 0 ; word[i] != '\0' ; i++)
    {
        hash = 31*hash + word[i];
    }
    return hash % SIZE;
}
char *SubString(char* String, int Start, int End){
  char *Result = (char*)malloc(sizeof(char)*((End - Start)+1));
  for(int i=Start, j=0; i<End+1; i++, j++){
    Result[j] = String[i];
  }
  Result[End-Start] = 0;
  return Result;
}
AST_node JsonToAST(char * JsonString){
  AST_node Root = {0};
  Root.Type = HASH_ENTRY_DICTIONARY;
  bool FindObject = true;
  bool FindKey = false;
  bool FindValue = false;
  int KeyBeginIndex = -1;
  int KeyEndIndex   = -1;
  int ValueStartIndex = -1;
  int ValueEndIndex = -1;
  AST_node *CurrentNode = {0};
  char *Key;
  char *Current = JsonString;
  while(*Current){
    //putchar((int)*Current);
    if(FindObject && *Current == '{'){
      FindObject = false;
      FindKey = true;
    }
    if(FindKey && *Current == '}'){
      break;
    }
    if(FindKey){
      if(*Current == '\''){
        if(KeyBeginIndex < 0){
          KeyBeginIndex = Current - JsonString + 1;
        }else if(KeyEndIndex < 0){
          KeyEndIndex = Current - JsonString;
        }
      }
      else if(KeyEndIndex > 0 && KeyBeginIndex > 0){
          FindKey = false;
          FindValue = true;

          Key = SubString(JsonString, KeyBeginIndex, KeyEndIndex);
          KeyBeginIndex = -1;
          KeyEndIndex   = -1;

          Root.NumChildren++;
          Root.Children = (AST_node *)realloc(Root.Children, sizeof(AST_node)* (Root.NumChildren));
          CurrentNode = Root.Children + (Root.NumChildren - 1);

          CurrentNode->Key = Key;
          CurrentNode->NumChildren = 0;
          CurrentNode->Children = 0;
        }
    }
    if(FindValue){
      bool TypeString = false;
      bool TypeNumber = false;
      bool TypeBool = false;
      bool TypeDictionary = false;
      character_stack Stack = {0};
      Stack.Stack = (char*)malloc(sizeof(char)*500);
      Stack.MaxSize = 500;
      Stack.NextStackIndex = 0;
      char StringDel = '\'';
      while(ValueStartIndex < 0 && (*Current == ' ' || *Current == '\n' || *Current == ':' )){
        Current++;
      }
      if(ValueStartIndex < 0){
        ValueStartIndex = Current - JsonString;
      }
      if(*Current == '"' || *Current == '\''){
        StringDel = *Current;
        Current++;
        TypeString = true;
      }
      else if(*Current == 'f' || *Current == 't'){
        TypeBool = true;
      }else if(*Current == '{'){
        TypeDictionary = true;
      }
      else{
        TypeNumber = true;
      }
      if(TypeString){
        while(*Current != StringDel){
          Current++;
        }
        ValueEndIndex = Current - JsonString;
        CurrentNode->Type = HASH_ENTRY_STRING;
        CurrentNode->Value = SubString(JsonString, ValueStartIndex, ValueEndIndex);
      }
      if(TypeNumber){
        while(*Current != ','){
          Current++;
        }
        ValueEndIndex = Current - JsonString;
        CurrentNode->Type = HASH_ENTRY_FLOAT;
        double *Val = (double *)malloc(sizeof(double));
        *Val = atof(SubString(JsonString, ValueStartIndex, ValueEndIndex));
        CurrentNode->Value = Val;
      }
      if(TypeBool){
        CurrentNode->Type = HASH_ENTRY_BOOL;
        bool BooleanValue = true;
        if(*Current == 'f'){
          BooleanValue = false;
        }else if(*Current == 't'){
          BooleanValue = true;
        }
          bool *Val = (bool *)malloc(sizeof(bool));
          *Val = BooleanValue;
          CurrentNode->Value = Val;
      }
      if(TypeDictionary){
        //Find the end of the object.
        while(*Current ){
          if(*Current == '}'){
            Pop(&Stack);
            if(!Peek(&Stack)){
              break;
            }
          }
          if(*Current == '{'){
            Push(&Stack, '{');
          }
          Current++;
        }
        ValueEndIndex = (Current - JsonString) + 1;
        char *Dictionary = SubString(JsonString, ValueStartIndex, ValueEndIndex);
        AST_node Child = JsonToAST(Dictionary);
        CurrentNode->Type = HASH_ENTRY_DICTIONARY;
        CurrentNode->NumChildren++;
        CurrentNode->Children = (AST_node *)realloc(CurrentNode->Children, sizeof(AST_node)*CurrentNode->NumChildren);
        AST_node *ChildSlot = CurrentNode->Children + (CurrentNode->NumChildren - 1);
        ChildSlot->Key = Child.Key;
        ChildSlot->Value = Child.Value;
        ChildSlot->Children = Child.Children;
        ChildSlot->Type = Child.Type;
        ChildSlot->NumChildren = Child.NumChildren;
      }

      ValueStartIndex = -1;
      ValueEndIndex = -1;
      FindValue = false;
      FindKey = true;
    }
    Current++;
  }
  return Root;
}

hash_table ParseJson(char *JsonString){
  hash_table Result = {0};
  character_stack Stack = {0};
  Stack.Stack = (char*)malloc(sizeof(char)*500);
  Stack.MaxSize = 500;
  Stack.NextStackIndex = 0;
  Result.Entries = (hash_entry *)malloc(sizeof(hash_entry)* 499);
  Result.Length = 0;
  
  // This is going to parse the JSON into an AST
  // Then it will be easy to walk the tree and fill out the hash
  //             [root]
  //             /   \
  //            /     \
  //           /       \
  //          /         \
  //         /           \
  //        /             \
  //      [key]          [key-value-type]
  //      /  \            / \
  //     /    \          /   \
  //  [type] [value(s)] [type] [value(s)]
  //             /\               \
  // This will only work for JSON that looks like this {'key':'value'} not things that start will arrays [{'okay':'yeh'}, 1, 2, 3]
  // As to why I didnt use a multi-line comment here. I couldn't tell you!
 AST_node AST = JsonToAST(JsonString);
  return Result;
}

int main(){
  char *json = "{\n\
                'that':2,\n\
                'this':2.2,\n\
                'child' : 'hey', 'that': true, 'dat': {'ass':'AWESOME', 'reddit': {'dot': 'com'}}}";
  ParseJson(json);

}