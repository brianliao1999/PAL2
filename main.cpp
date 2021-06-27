
# include <iostream>
# include <iomanip>
# include <stdio.h>
# include <stdlib.h>
# include <string>
# include <string.h>
# include <vector>

using namespace std ;

// ---       class head        ---

// An object of a Token that contain every data about Token include the Token it point to,
// linked by pointer( link list ).
class Token ;

// An object which has every data about s-expression.
class S_Expression ;

// An object which has every data about error.
class Error ;

// An object which contain a Token pointer and a left-node pointer and a right-node pointer
class CorrespondingTree ;

// An object to bind with a Corresponding tree
class Symbol ;

// An object which has a job to get input in.
class Scanner ;

// An object which has jobs to check syntax and evaluate.
class Parser ;

// ---       type head         ---



// ---       type define       ---

// A type of Token.
enum TokenType { LEFTPAREN, RIGHTPAREN, INT, STRING, DOT, FLOAT,
                 NIL, T, QUOTE, SYMBOL, SEMICOLON, DEFAULTTOKEN
} ;

// A type of Error.
enum ErrorType { HASEOF, EXPECTRIGHT, EXPECTLEFT, NOCLOSE, OTHERS, DEFAULTERROR,
                 UNBOUND, NONLIST, APPLYNONFUNC, LEVEL, FORMAT, ARGUMENTS,
                 NONFUNCTION, ARGTYPE, NOVALUE, DIVIDEBYZERO
} ;



typedef Token * TokenPtr ;

typedef string * StringPtr ;

typedef char * CharPtr ;

typedef vector<Error> * ErrorVctPtr ;

typedef vector<Symbol> * SymbolVctPtr ;

typedef S_Expression * SExpressionPtr ;

typedef CorrespondingTree * CorrespondingTreePtr ;

// ---       function head     ---

// ---       class             ---

class Token {
    
public:
  TokenType mTokenType ;
  CharPtr mToken ;
  int mLine ;
  int mColumn ;
  TokenPtr mNext ;
    
  Token() { // Constructor
    mTokenType = DEFAULTTOKEN ;
    mToken = NULL ;
    mLine = -1 ;
    mColumn = -1 ;
    mNext = NULL ;
  } // Token()
  
  static void DeleteTokenString( TokenPtr & head ) {
    
    if ( head == NULL ) {
      return ;
    } // if
    else if ( head->mNext == NULL ) {
      delete [] head->mToken ;
      head->mToken = NULL ;
      
      return ;
    } // else if
    else {
      DeleteTokenString( head->mNext ) ;
      delete [] head->mToken ;
      head->mToken = NULL ;
      
      return ;
    } // else
    
  } // DeleteTokenString()
    
} ; //  class Token

class S_Expression {
    
public:
  TokenPtr mTokenString ;
    
  S_Expression() { // Constructor
    mTokenString = NULL ;
        
    return ;
  } // S_Expression()
  
  static void DeleteSExp( SExpressionPtr & sExp  ) {
    
    sExp->mTokenString = NULL ;
    
  } // DeleteSExp()
    
} ; // class S_Expression

class Error {
    
public:
  ErrorType mErrorType ;
  int mLine ;
  int mColumn ;
  string mToken ;
  CorrespondingTreePtr mBinding ;
    
  Error() { // Constructor
    mErrorType = DEFAULTERROR ;
    mLine = -1 ;
    mColumn = -1 ;
    mToken.clear() ;
    mBinding = NULL ;
        
    return ;
  } // Error()
    
} ; // class Error

class CorrespondingTree {
  
public:
  TokenPtr mToken ;
  CorrespondingTreePtr mLeftNode ;
  CorrespondingTreePtr mRightNode ;
  
  CorrespondingTree() { // Constructor
    mToken = NULL ;
    mLeftNode = NULL ;
    mRightNode = NULL ;
    
  } // CorrespondingTree()
  
} ; // class CorrespondingTree

class Symbol {
  
public:
  CorrespondingTreePtr mSymbol ;
  CorrespondingTreePtr mBinding ;
  
  Symbol() { // Constructor
    mSymbol = NULL ;
    mBinding = NULL ;
    
    return ;
  } // Symbol()
  
} ; // class Symbol

class Scanner {
  StringPtr mLoadedLine ;
  TokenPtr mPeekedToken ;
  ErrorVctPtr mErrorVct ;
  int mLine ;
  int mColumn ;
  
public:
  Scanner() { // Constructor
    mLoadedLine = new string ;
    mLoadedLine->clear() ;
    mPeekedToken = NULL ;
    mErrorVct = new vector<Error> ;
    mErrorVct->clear() ;
    mLine = -1 ;
    mColumn = 0 ;
        
    return ;
  } // Scanner()
    
  // A function which has to read in a bunch of Token until them can
  // be combined to a S-Expression or there are errors.
  // get a line per times.
  bool ReadSExp( SExpressionPtr & sExp ) {
    sExp = new S_Expression ;
    bool error = false ;
    sExp->mTokenString = GetSExp( error, true ) ;
    mColumn = 0 ;
    
    if ( AllWhiteSpace( mLoadedLine ) || IsComment( mLoadedLine ) ) {
      
      mLoadedLine->clear() ;
    } // if
    
    if ( mLoadedLine->empty()  ) {
      mLine = 0 ;
    } // if
    else {
      mLine = 1 ;
    } // else (  )
    
    return ! error ;
  } // ReadSExp()
  
  bool AllWhiteSpace( StringPtr string ) {
    
    int len = string->size() ;
    
    for ( int i = 0 ; i < len ; i++ ) {
      if ( string->at( i ) != ' ' && string->at( i ) != '\t' && string->at( i ) != '\n' ) {
        return false ;
      } // if
      
    } // for
    
    return true ;
  } // AllWhiteSpace()
  
  bool IsComment( StringPtr string ) {
    
    int len = string->size() ;
    int i = 0 ;
    
    for ( i = 0 ; i < len && ( string->at( i ) == ' ' ||
                               string->at( i ) == '\t' || string->at( i ) == '\n' ) ; i++ ) {
      ;
    } // for
    
    if ( i < len && string->at( i ) == ';' )
      return true ;
    
    return false ;
  } // IsComment()
  
  // A function which has to get a S-Expression( Token string )
  TokenPtr GetSExp( bool & hasError, bool first ) {
    
    // <S-exp> :: = <ATOM> | LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN | QUOTE <S-exp>
    
    TokenPtr head = NULL ;
    TokenPtr tail = NULL ;
    TokenType mPeekedTokenType = DEFAULTTOKEN ;
    
    if ( GetToken( head ) ) {
      // get a Token
      
      if ( IsAtom( head->mTokenType ) ) {
        // :: = <ATOM>
        // the Token is an ATOM, also a S-EXP
        
        hasError = false ;
        
        return head ;
      } // if
      else if ( head->mTokenType == LEFTPAREN ) {
        // :: = <ATOM> | LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN
        // the Token is a LEFT-PAREN, could be an ATOM or sth else.
        
        if ( PeekToken( mPeekedTokenType ) ) {
          // peek what next Token is
          
          if ( mPeekedTokenType == RIGHTPAREN ) {
            // :: = <ATOM> :: = RIGHT-PAREN LEFT-PAREN
            // the Token string could be combined to an ATOM
            // also a S-EXP
            
            GetToken( head->mNext ) ;
            hasError = false ;
            
            return head ;
          } // if
          else {
            // :: = LEFT-PAREN !at here! <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN
            // it couldn't to be an ATOM, should be sth else.
            // and the next should be a S-EXP or DOT or RIGHT-PAREN
            
            head->mNext = GetSExp( hasError, false ) ;
            if ( hasError ) {
              // there are some errors when getting next S-EXP.
              
              DeleteTokenPtr( head ) ;
              
              return head ;
            } // if
            else { // if ( no error )
              tail = GetTail( head ) ;
              
              // :: = LEFT-PAREN <S-exp> !at here! { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN
              // have got a S-EXP, peek what next Token is.
              
              if ( PeekToken( mPeekedTokenType ) ) {
                
                while ( mPeekedTokenType != DOT && mPeekedTokenType != RIGHTPAREN && ! hasError ) {
                  // the next Token isn't a DOT or RIGHT-PAREN
                  // there are no errors when peeking and getting next S-EXP
                  // continue with getting S-EXP
                  // until there has some errors or the next Token
                  // is DO or RIGHT-PAREN
                  
                  tail->mNext = GetSExp( hasError, false ) ;
                  tail = GetTail( head ) ;
                  if ( ! hasError ) {
                    hasError = ! PeekToken( mPeekedTokenType ) ;
                  } // if
                  
                } // while
                
                // :: = LEFT-PAREN <S-exp> { <S-exp> } !at here! [ DOT <S-exp> ] RIGHT-PAREN
                // there has some errors or the next Token
                // is DOT or RIGHT-PAREN
                
                if ( hasError ) {
                  // there has some errors
                  
                  DeleteTokenPtr( head ) ;
                  
                  return head ;
                } // if
                else if ( mPeekedTokenType == DOT ) {
                  // the next Token is DOT
                  
                  GetToken( tail->mNext ) ;
                  tail = tail->mNext ;
                  
                  // :: = LEFT-PAREN <S-exp> { <S-exp> } [ DOT !at here! <S-exp> ] RIGHT-PAREN
                  // next should be a S-EXP or RIGHT-PAREN
                  
                  tail->mNext = GetSExp( hasError, false ) ;
                  
                  if ( hasError ) {
                    // there has some errors.
                    
                    DeleteTokenPtr( head ) ;
                    
                    return head ;
                  } // if
                  else { // if ( no error )
                    tail = GetTail( tail ) ;
                    // :: = LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] !at here! RIGHT-PAREN
                    // the next Token should be RIGHT-PAREN
                    
                    if ( GetToken( tail->mNext ) ) {
                      // have got next Token.
                      tail = tail->mNext ;
                      
                      
                      if ( tail->mTokenType == RIGHTPAREN ) {
                        // :: = LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN !at here!
                        // it is RIGHT-PAREN !
                        // the Token string is a S-EXP !
                        
                        hasError = false ;
                        
                        return head ;
                      } // if
                      else {
                        // here should has a right-paren, but it doesn't
                        // so it should be an error
                        
                        Error temp ;
                        temp.mLine = tail->mLine ;
                        temp.mColumn = tail->mColumn ;
                        temp.mToken = * tail->mToken ;
                        temp.mErrorType = EXPECTRIGHT ;
                        mErrorVct->push_back( temp ) ;
                        
                        hasError = true ;
                        DeleteTokenPtr( head ) ;
                        
                        return head ;
                      } // else
                      
                    } // if
                    else {
                      // next Token has some errors
                      hasError = true ;
                      DeleteTokenPtr( head ) ;
                      
                      return head ;
                    } // else
                      
                  } // else
                  
                } // if
                else if ( mPeekedTokenType == RIGHTPAREN ) {
                  // :: = LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN !at here!
                  // the Token string is a S-exp !
                  
                  GetToken( tail->mNext ) ;
                  
                  hasError = false ;
                  
                  return head ;
                } // else if
                else { // if ( mPeekedTokenType != DOT && mPeekedTokenType != RIGHTPAREN )
                  // :: = LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] !at here! RIGHT-PAREN
                  // here should has a RIGHT-PAREN, but it doesn't
                  // so it should be an error.
                  
                  Error temp ;
                  temp.mLine = tail->mLine ;
                  temp.mColumn = tail->mColumn ;
                  temp.mToken = * tail->mToken ;
                  temp.mErrorType = EXPECTRIGHT ;
                  mErrorVct->push_back( temp ) ;
                  
                  hasError = true ;
                  DeleteTokenPtr( head ) ;
                  
                  return head ;
                } // else
                
              } // if
              else {
                // next Token has some errors
                hasError = true ;
                DeleteTokenPtr( head ) ;
                
                return head ;
              } // else
              
            } // else
            
          } // else
          
        } // if
        else {
          // next Token has some errors
          hasError = true ;
          DeleteTokenPtr( head ) ;
          
          return head ;
        } // else
        
      } // else if
      else if ( head->mTokenType == QUOTE ) {
        // :: = QUOTE <S-exp>
        // the next should be a S-EXP
        
        head->mNext = GetSExp( hasError, false ) ;
        if ( hasError ) {
          // there has some errors
          
          DeleteTokenPtr( head ) ;
          
          return head ;
        } // if
        else { // if ( no error )
          // it is a S-EXP !
          // the Token string is a S-EXP !
          
          tail = GetTail( head ) ;
          hasError = false ;
          
          return head ;
        } // else
        
      } // else if
      else {
        // S-Expression should has <ATOM> or LAFT-PAREN or QUOTE to be first Token
        // but it isn't, so it should be an error.
        
        Error temp ;
        temp.mLine = head->mLine ;
        temp.mColumn = head->mColumn ;
        temp.mToken = * head->mToken ;
        temp.mErrorType = EXPECTLEFT ;
        mErrorVct->push_back( temp ) ;
        
        hasError = true ;
        DeleteTokenPtr( head ) ;
        
        return head ;
      } // else
      
    } // if
    else {
      // next Token has some errors
      
      DeleteTokenPtr( head ) ;
      
      hasError = true ;
      
      return head ;
    } // else
    
  } // GetSExp()
  
  TokenPtr GetTail( TokenPtr head ) {
    if ( head->mNext == NULL )
      return head ;
    else
      return GetTail( head->mNext ) ;
      
  } // GetTail()
  
  void DeleteTokenPtr( TokenPtr & head ) {
    if ( head == NULL )
      return ;
    else if ( head->mNext == NULL ) {
      delete head ;
      head = NULL ;
      
      return ;
    } // else if
    else {
      DeleteTokenPtr( head->mNext ) ;
      delete head ;
      head = NULL ;
      
      return ;
    } // else
    
  } // DeleteTokenPtr()
    
  // A function which has to get a Token and return a TokenPtr that
  // point to a Token contain a string of this Token and
  // its TokenType and line and column.
  bool GetToken( TokenPtr & token ) {
    int i = 0 ;
    int j = 0 ;
    int index = 0 ;
    StringPtr temp = new string ;
    bool hasError = false ;
    
    if ( mPeekedToken != NULL ) {
      token = mPeekedToken ;
      mPeekedToken = NULL ;
      
      return true ;
    } // if
    else { // if ( mPeekedToken == NULL )
      
      while ( mLoadedLine->empty() && ! cin.eof() ) {
        mLoadedLine = GetLine() ;
      } // while
      
      if ( ! mLoadedLine->empty() ) {
        token = new Token ;
        
        if ( GetString( index, temp, hasError ) ) {
          mColumn = mColumn + index + 1 ;
          token->mToken = new char[ temp->size() + 1 ] ;
          for ( j = 0 ; j < temp->size() ; j++ ) {
            token->mToken[ j ] = temp->at( j ) ;
          } // for
          
          token->mToken[ j ] = '\0' ;
          
          delete temp ;
          temp = NULL ;
          token->mLine = mLine ;
          token->mColumn = mColumn ;
          token->mTokenType = STRING ;
          
          mLoadedLine->erase( mLoadedLine->begin(), mLoadedLine->begin() + index + 1 ) ;
          
          return true ;
        } // if
        else if ( hasError ) {
          DeleteTokenPtr( mPeekedToken ) ;
          
          return false ;
        } // else if
        else if ( mLoadedLine->at( 0 ) == ';' ) {
          temp->clear() ;
          temp = NULL ;
          token = NULL ;
          
          mLoadedLine->clear() ;
          
          
          return GetToken( token ) ;
        } // else if
        else if ( mLoadedLine->at( 0 ) == '\'' ) {
          mColumn = mColumn + 1 ;
          temp->clear() ;
          temp->push_back( '\'' ) ;
          token->mToken = new char[ temp->size() + 1 ] ;
          for ( j = 0 ; j < temp->size() ; j++ ) {
            token->mToken[ j ] = temp->at( j ) ;
          } // for
          
          token->mToken[ j ] = '\0' ;
          
          delete temp ;
          temp = NULL ;
          token->mLine = mLine ;
          token->mColumn = mColumn ;
          token->mTokenType = QUOTE ;
          
          mLoadedLine->erase( mLoadedLine->begin() ) ;
          
          return true ;
        } // else if
        else if ( mLoadedLine->at( 0 ) == '(' ) {
          mColumn = mColumn + 1 ;
          temp->clear() ;
          temp->push_back( '(' ) ;
          token->mToken = new char[ temp->size() + 1 ] ;
          for ( j = 0 ; j < temp->size() ; j++ ) {
            token->mToken[ j ] = temp->at( j ) ;
          } // for
          
          token->mToken[ j ] = '\0' ;
          
          delete temp ;
          temp = NULL ;
          token->mLine = mLine ;
          token->mColumn = mColumn ;
          token->mTokenType = LEFTPAREN ;
          
          mLoadedLine->erase( mLoadedLine->begin() ) ;
          
          return true ;
        } // else if
        else if ( mLoadedLine->at( 0 ) == ')' ) {
          mColumn = mColumn + 1 ;
          temp->clear() ;
          temp->push_back( ')' ) ;
          token->mToken = new char[ temp->size() + 1 ] ;
          for ( j = 0 ; j < temp->size() ; j++ ) {
            token->mToken[ j ] = temp->at( j ) ;
          } // for
          
          token->mToken[ j ] = '\0' ;
          
          delete temp ;
          temp = NULL ;
          token->mLine = mLine ;
          token->mColumn = mColumn ;
          token->mTokenType = RIGHTPAREN ;
          
          mLoadedLine->erase( mLoadedLine->begin() ) ;
          
          return true ;
        } // else if
        else if ( mLoadedLine->at( 0 ) == ' ' ||
                  mLoadedLine->at( 0 ) == '\t' || mLoadedLine->at( 0 ) == '\n' ) {
          
          mLoadedLine->erase( mLoadedLine->begin() ) ;
          mColumn++ ;
          
          return GetToken( token ) ;
        } // else if
        else {
          while ( temp->empty() && ! mLoadedLine->empty() ) {
            for ( i = 0 ; i < mLoadedLine->size() &&
                  mLoadedLine->at( i ) != '(' &&
                  mLoadedLine->at( i ) != ')' &&
                  mLoadedLine->at( i ) != '\'' &&
                  mLoadedLine->at( i ) != '\"' &&
                  mLoadedLine->at( i ) != ' ' &&
                  mLoadedLine->at( i ) != '\t' &&
                  mLoadedLine->at( i ) != '\n' &&
                  mLoadedLine->at( i ) != ';' ; i++ ) {
              temp->push_back( mLoadedLine->at( i ) ) ;
              
            } // for
            
            mLoadedLine->erase( mLoadedLine->begin(), mLoadedLine->begin() + i ) ;
            
            if ( temp->empty() && ! mLoadedLine->empty() ) {
              mLoadedLine->erase( mLoadedLine->begin() ) ;
              mColumn++ ;
            } // if
            
          } // while
          
          if ( temp->empty() ) { // mLoadedLine is empty
            return GetToken( token ) ;
          } // if
          else { // temp isn't empty
            if ( IsT( temp ) ) {
              mColumn = mColumn + temp->size() ;
              token->mToken = new char[ temp->size() + 1 ] ;
              for ( j = 0 ; j < temp->size() ; j++ ) {
                token->mToken[ j ] = temp->at( j ) ;
              } // for
              
              token->mToken[ j ] = '\0' ;
              
              delete temp ;
              temp = NULL ;
              token->mLine = mLine ;
              token->mColumn = mColumn ;
              token->mTokenType = T ;
              
              return true ;
            } // if
            else if ( IsDot( temp ) ) {
              mColumn = mColumn + temp->size() ;
              token->mToken = new char[ temp->size() + 1 ] ;
              for ( j = 0 ; j < temp->size() ; j++ ) {
                token->mToken[ j ] = temp->at( j ) ;
              } // for
              
              token->mToken[ j ] = '\0' ;
              
              delete temp ;
              temp = NULL ;
              token->mLine = mLine ;
              token->mColumn = mColumn ;
              token->mTokenType = DOT ;
              
              return true ;
            } // else if
            else if ( IsNIL( temp ) ) {
              mColumn = mColumn + temp->size() ;
              token->mToken = new char[ temp->size() + 1 ] ;
              for ( j = 0 ; j < temp->size() ; j++ ) {
                token->mToken[ j ] = temp->at( j ) ;
              } // for
              
              token->mToken[ j ] = '\0' ;
              
              delete temp ;
              temp = NULL ;
              token->mLine = mLine ;
              token->mColumn = mColumn ;
              token->mTokenType = NIL ;
              
              return true ;
            } // else if
            else if ( IsInteger( temp ) ) {
              mColumn = mColumn + temp->size() ;
              token->mToken = new char[ temp->size() + 1 ] ;
              for ( j = 0 ; j < temp->size() ; j++ ) {
                token->mToken[ j ] = temp->at( j ) ;
              } // for
              
              token->mToken[ j ] = '\0' ;
              
              delete temp ;
              temp = NULL ;
              token->mLine = mLine ;
              token->mColumn = mColumn ;
              token->mTokenType = INT ;
              
              return true ;
            } // else if
            else if ( IsFloat( temp ) ) {
              mColumn = mColumn + temp->size() ;
              token->mToken = new char[ temp->size() + 1 ] ;
              for ( j = 0 ; j < temp->size() ; j++ ) {
                token->mToken[ j ] = temp->at( j ) ;
              } // for
              
              token->mToken[ j ] = '\0' ;
              
              delete temp ;
              temp = NULL ;
              token->mLine = mLine ;
              token->mColumn = mColumn ;
              token->mTokenType = FLOAT ;
              
              return true ;
            } // else if
            else {
              mColumn = mColumn + temp->size() ;
              token->mToken = new char[ temp->size() + 1 ] ;
              for ( j = 0 ; j < temp->size() ; j++ ) {
                token->mToken[ j ] = temp->at( j ) ;
              } // for
              
              token->mToken[ j ] = '\0' ;
              
              delete temp ;
              temp = NULL ;
              token->mLine = mLine ;
              token->mColumn = mColumn ;
              token->mTokenType = SYMBOL ;
              
              return true ;
            } // else
            
          } // else
          
        } // else
        
      } // if
      else {
        Error temp ;
        temp.mErrorType = HASEOF ;
        temp.mColumn = mColumn ;
        temp.mLine = mLine ;
        mErrorVct->push_back( temp ) ;
        
        return false ;
      } // else
      
    } // else
    
  } // GetToken()
  
  // A function to peek what next Token is ane use GetToken() to get a Token
  // and put it into mPeekedToken
  bool PeekToken( TokenType & tokenType ) {
    if ( mPeekedToken != NULL ) {
      
      tokenType = mPeekedToken->mTokenType ;
      
      return true ;
    } // if
    else {
      
      TokenPtr temp ;
      bool noError = GetToken( temp ) ;
      
      if ( noError ) {
        mPeekedToken = temp ;
        tokenType = mPeekedToken->mTokenType ;
        temp = NULL ;

        return true ;
      } // if
      else { // if ( hasError )
        DeleteTokenPtr( temp ) ;
        
        return false ;
      } // else
      
    } // else
    
  } // PeekToken()
    
  StringPtr GetLine() {
    StringPtr lineIn = new string ;
    char temp ;
        
    temp = cin.get() ;
    while ( temp != '\n' && ! cin.eof() ) {
      lineIn->push_back( temp ) ;
      temp = cin.get() ;
    } // while
    
    mLine++ ;
    mColumn = 0 ;
        
    return lineIn ;
  } // GetLine()
  
  // return true if next Token is a string
  // and return its index in mLoadedLine
  // else return false.
  // must be start at index 0.
  bool GetString( int & index, StringPtr string, bool & hasError ) {
    int escape = 0 ;
    index = 0 ;
    hasError = false ;
    
    if ( mLoadedLine->at( index ) == '\"' ) {
      
      for (  index = 1 ; index < mLoadedLine->size() ; index++ ) {
        if ( mLoadedLine->at( index ) == '\\' ) {
          for ( int j = index ; index < mLoadedLine->size() && mLoadedLine->at( j ) == '\\' ; j++ ) {
            escape++ ;
          } // for
          
          for ( int k = escape / 2 ; k > 0 ; k-- ) {
            string->push_back( '\\' ) ;
          } // for
          
          index = index + escape ;
          
          if ( escape % 2 == 1 ) {
            if ( index < mLoadedLine->size() ) {
              if ( mLoadedLine->at( index ) == 'n' ) {
                string->push_back( '\n' ) ;
                
              } // if
              else if ( mLoadedLine->at( index ) == 't' ) {
                string->push_back( '\t' ) ;
              } // else if
              else if ( mLoadedLine->at( index ) == '"' ) {
                string->push_back( '\"' ) ;
              } // else if
              else {
                index-- ;
                string->push_back( '\\' ) ;
              } // else
              
            } // if
            else {
              string->push_back( '\\' ) ;
            } // else
            
          } // if
          else {
            index-- ;
          } // else
          
          escape = 0 ;
          
        } // if
        else if ( mLoadedLine->at( index ) == '\"' ) {
          
          return true ;
        } // else if
        else {
          string->push_back( mLoadedLine->at( index ) ) ;
        } // else
        
      } // for
      
      hasError = true ;
      Error temp ;
      temp.mLine = mLine ;
      temp.mColumn = mColumn + index + 1 ;
      temp.mErrorType = NOCLOSE ;
      
      mErrorVct->push_back( temp ) ;
      
      string->clear() ;
      mLoadedLine->clear() ;
      mLine = 0 ;
      mColumn = 0 ;
      
      return false ;
    } // if
    else
      return false ;
    
  } // GetString()
  
  // A function to check if a string is a Token whose Token type is Integer.
  bool IsInteger( StringPtr string ) {
    
    if ( string->at( 0 ) != '0' && string->at( 0 ) != '1' &&
         string->at( 0 ) != '2' && string->at( 0 ) != '3' &&
         string->at( 0 ) != '4' && string->at( 0 ) != '5' &&
         string->at( 0 ) != '6' && string->at( 0 ) != '7' &&
         string->at( 0 ) != '8' && string->at( 0 ) != '9' &&
         string->at( 0 ) != '+' && string->at( 0 ) != '-' ) {
      
      return false ;
    } // if
    
    for ( int i = 1 ; i < string->size() ; i++ ) {
      if ( string->at( i ) != '0' && string->at( i ) != '1' &&
           string->at( i ) != '2' && string->at( i ) != '3' &&
           string->at( i ) != '4' && string->at( i ) != '5' &&
           string->at( i ) != '6' && string->at( i ) != '7' &&
           string->at( i ) != '8' && string->at( i ) != '9' ) {
        
        return false ;
      } // if
      
    } // for
    
    if ( ( string->at( 0 ) == '+' || string->at( 0 ) == '-' ) && string->size() == 1 ) {
      
      return false ;
    } // if
    else {
      
      return true ;
    } // else
    
  } // IsInteger()
  
  // A function to check if a string is a Token whose Token type is Float.
  bool IsFloat( StringPtr string ) {
    int i = 0 ;
    bool hasDot = false ;
    bool hasDigit = false ;
    
    if ( string->at( 0 ) != '0' && string->at( 0 ) != '1' &&
         string->at( 0 ) != '2' && string->at( 0 ) != '3' &&
         string->at( 0 ) != '4' && string->at( 0 ) != '5' &&
         string->at( 0 ) != '6' && string->at( 0 ) != '7' &&
         string->at( 0 ) != '8' && string->at( 0 ) != '9' &&
         string->at( 0 ) != '+' && string->at( 0 ) != '-' && string->at( 0 ) != '.' ) {
      
      return false ;
    } // if
    
    hasDigit = ( string->at( 0 ) == '0' || string->at( 0 ) == '1' ||
                 string->at( 0 ) == '2' || string->at( 0 ) == '3' ||
                 string->at( 0 ) == '4' || string->at( 0 ) == '5' ||
                 string->at( 0 ) == '6' || string->at( 0 ) == '7' ||
                 string->at( 0 ) == '8' || string->at( 0 ) == '9' ) ;
    
    if ( string->at( 0 ) == '.' ) {
      hasDot = true ;
    } // if
    
    for ( i = 1 ; i < string->size() ; i++ ) {
      if ( string->at( i ) != '0' && string->at( i ) != '1' &&
           string->at( i ) != '2' && string->at( i ) != '3' &&
           string->at( i ) != '4' && string->at( i ) != '5' &&
           string->at( i ) != '6' && string->at( i ) != '7' &&
           string->at( i ) != '8' && string->at( i ) != '9' && string->at( i ) != '.' ) {
        
        return false ;
      } // if
      
      if ( ! hasDigit ) {
        hasDigit = ( string->at( i ) == '0' || string->at( i ) == '1' ||
                     string->at( i ) == '2' || string->at( i ) == '3' ||
                     string->at( i ) == '4' || string->at( i ) == '5' ||
                     string->at( i ) == '6' || string->at( i ) == '7' ||
                     string->at( i ) == '8' || string->at( i ) == '9' ) ;
      } // if
      
      if ( string->at( i ) == '.' ) {
        if ( hasDot ) {
          return false ;
        } // if
        else {
          hasDot = true ;
        } // else
        
      } // if
      
    } // for
    
    return hasDigit ;
  } // IsFloat()
  
  bool IsNIL( StringPtr string ) {
    
    if ( string->size() == 3 && string->at( 0 ) == 'n' &&
         string->at( 1 ) == 'i' && string->at( 2 ) == 'l' ) {
      
      return true ;
    } // if
    else if ( string->size() == 2 && string->at( 0 ) == '#' && string->at( 1 ) == 'f' ) {
      
      return true ;
    } // else if
    else {
      
      return false ;
    } // else
    
  } // IsNIL()
  
  bool IsT( StringPtr string ) {
    
    if ( string->size() == 1 && string->at( 0 ) == 't' ) {
      
      return true ;
    } // if
    else if ( string->size() == 2 && string->at( 0 ) == '#' && string->at( 1 ) == 't' ) {
      
      return true ;
    } // else if
    else {
      
      return false ;
    } // else
    
  } // IsT()
  
  bool IsDot( StringPtr string ) {
    if ( string->size() == 1 && string->at( 0 ) == '.' ) {
      
      return true ;
    } // if
    else {
      
      return false ;
    } // else
    
  } // IsDot()
    
  void PrintError( bool & hasEof ) {
    for ( int i = 0 ; i < mErrorVct->size() ; i++ ) {
      if ( mErrorVct->at( i ).mErrorType == HASEOF &&
           ( mLoadedLine->empty() || AllWhiteSpace( mLoadedLine ) ) ) {
        cout << "ERROR (no more input) : END-OF-FILE encountered" << endl ;
        hasEof = true ;
      } // if
      else if ( mErrorVct->at( i ).mErrorType == NOCLOSE ) {
        cout << "ERROR (no closing quote) : END-OF-LINE encountered at Line " <<
                mErrorVct->at( i ).mLine << " Column " << mErrorVct->at( i ).mColumn << endl ;
      } // else if
      else if ( mErrorVct->at( i ).mErrorType == EXPECTRIGHT ) {
        cout << "ERROR (unexpected token) : ')' expected when token at Line " <<
                mErrorVct->at( i ).mLine << " Column " << mErrorVct->at( i ).mColumn <<
                " is >>" << mErrorVct->at( i ).mToken << "<<" << endl ;
      } // else if
      else if ( mErrorVct->at( i ).mErrorType == EXPECTLEFT ) {
        cout << "ERROR (unexpected token) : atom or '(' expected when token at Line " <<
                mErrorVct->at( i ).mLine << " Column " << mErrorVct->at( i ).mColumn <<
                " is >>" << mErrorVct->at( i ).mToken << "<<" << endl ;
      } // else if
      else {
        cout << "ERROR (unexpected error)" << endl ;
      } // else
      
    } // for
    
    mErrorVct->clear() ;
    mLoadedLine->clear() ;
    mLine = 0 ;
    mColumn = 0 ;
    
    return ;
  } // PrintError()
  
  static bool IsAtom( TokenType tokenType ) {
    if ( tokenType == SYMBOL || tokenType == INT ||
         tokenType == FLOAT || tokenType == STRING ||
         tokenType == NIL || tokenType == T ) {
      
      return true ;
    } // if
    else {
      
      return false ;
    } // else
      
  } // IsAtom()
    
} ; // class Scanner

class Parser {
  ErrorVctPtr mErrorVct ;
  SymbolVctPtr mSymbolVct ;
  
public:
  Parser() { // Constructor
    mErrorVct = new vector<Error> ;
    mErrorVct->clear() ;
    mSymbolVct = new vector<Symbol> ;
    mSymbolVct->clear() ;
        
    return ;
  } // Parser()
    
  bool PrintSExp( SExpressionPtr sExp ) {
    TokenPtr walk = sExp->mTokenString ;
    
    while ( walk != NULL ) {
      if ( walk->mTokenType == INT ) {
        cout << atoi( walk->mToken ) ;
      } // if
      else if ( walk->mTokenType == FLOAT ) {
        cout << atof( walk->mToken ) ;
      } // else if
      else if ( walk->mTokenType == NIL ) {
        cout << "nil" ;
      } // else if
      else if ( walk->mTokenType == DOT ) {
        cout << "." ;
      } // else if
      else if ( walk->mTokenType == RIGHTPAREN ) {
        cout << ")" ;
      } // else if
      else if ( walk->mTokenType == LEFTPAREN ) {
        cout << "(" ;
      } // else if
      else if ( walk->mTokenType == T ) {
        cout << "#t" ;
      } // else if
      else {
        cout << walk->mToken ;
      } // else
      
      cout << " " ;
      walk = walk->mNext ;
    } // while
    
    cout << endl << endl ;
    
    return true ;
  } // PrintSExp()
  
  bool PrintCorrespondingTree( CorrespondingTreePtr head, int space, bool fromLeft ) {
    
    if ( head == NULL ) {
      return true ;
    } // if
    else { // if ( head != NULL )
      
      if ( head->mToken != NULL ) {
        
        PrintToken( head->mToken ) ;
        
        return true ;
      } // if
      else {
        
        if ( fromLeft ) {
          cout << "( " ;
          space = space + 2 ;

        } // if
        
        PrintCorrespondingTree( head->mLeftNode, space, true ) ;
        
        if ( head->mRightNode->mToken != NULL ) {
          if ( head->mRightNode->mToken->mTokenType != NIL ) {
            PrintSpace( space ) ;
            cout << "." << endl ;
            PrintSpace( space ) ;
            PrintToken( head->mRightNode->mToken ) ;
          } // if
          
        } // if
        else {
          PrintSpace( space ) ;
          PrintCorrespondingTree( head->mRightNode, space, false ) ;
        } // else
        
        if ( fromLeft ) {
          space = space - 2 ;
          PrintSpace( space ) ;
          cout << ")" << endl ;
        } // if
        
        return true ;
      } // else
      
    } // else
    
  } // PrintCorrespondingTree()
  
  void PrintToken( TokenPtr token ) {
    
    if ( token->mTokenType == DOT ) {
      cout << "." << endl ;
    } // if
    else if ( token->mTokenType == QUOTE ) {
      cout << "quote" << endl ;
    } // else if
    else if ( token->mTokenType == NIL ) {
      cout << "nil" << endl ;
    } // if
    else if ( token->mTokenType == T ) {
      cout << "#t" << endl ;
    } // else if
    else if ( token->mTokenType == INT ) {
      cout << atoi( token->mToken ) << endl ;
    } // else if
    else if ( token->mTokenType == FLOAT ) {
      cout << atof( token->mToken ) << endl ;
    } // else if
    else if ( token->mTokenType == STRING ) {
      cout << "\"" << token->mToken << "\"" << endl ;
    } // else if
    else if ( token->mTokenType != NIL ) {
      cout << token->mToken << endl ;
    } // else if
    
  } // PrintToken()
  
  void CheckTree( CorrespondingTreePtr head ) {
    
    if ( head != NULL ) {
      
      if ( head->mToken != NULL ) {
        if ( head->mToken->mTokenType == NIL ) {
          cout << "nil" << endl ;
        } // if
        else if ( head->mToken->mTokenType == DOT ) {
          cout << "." << endl ;
        } // else if
        else if ( head->mToken->mTokenType == QUOTE ) {
          cout << "quote" << endl ;
        } // else if
        else if ( head->mToken->mTokenType == T ) {
          cout << "#t" << endl ;
        } // else if
        else if ( head->mToken->mTokenType == INT ) {
          cout << atoi( head->mToken->mToken ) << endl ;
        } // else if
        else if ( head->mToken->mTokenType == FLOAT ) {
          cout << atof( head->mToken->mToken ) << endl ;
        } // else if
        else {
          cout << head->mToken->mToken << endl ;
        } // else
        
      } // if
      else {
        cout << "L : " << endl ;
        CheckTree( head->mLeftNode ) ;
        cout << "\nR : " << endl ;
        CheckTree( head->mRightNode ) ;
        
      } // else
      
      return ;
    } // if
    else {
      cout << "NULL" << endl ;
      return ;
    } // else
    
  } // CheckTree()
  
  void PrintSpace( int space ) {
    for ( int i = 0 ; i < space ; i++ ) {
      cout << " " ;
    } // for
    
    return ;
  } // PrintSpace()
  
  // up
  bool Eval( CorrespondingTreePtr head, CorrespondingTreePtr & value, int level ) {
    
    // an atom but not a symbol ( not a S-Expression )
    if ( head->mToken != NULL && head->mToken->mTokenType != SYMBOL &&
         Scanner::IsAtom( head->mToken->mTokenType ) ) {
      
      value = head ;
      
      return true ;
    } // if
    // a symbol but not a S-Expresssion
    else if ( head->mToken != NULL && head->mToken->mTokenType == SYMBOL ) {
      bool userDefined = false ;
      bool error = false ;
      CorrespondingTreePtr binding = GetBindingAndEval( head->mToken, userDefined, error, level ) ;
      if ( error ) {
        
        return false ;
      } // if
      
      if ( binding != NULL ) { // found the binding
        value = binding ;
        
        return true ;
      } // if
      else if ( IsPrimitiveFunc( head->mToken ) ) { // it's an internal function
        CorrespondingTreePtr temp = new CorrespondingTree ;
        temp->mToken = head->mToken ;
        char * tempC = temp->mToken->mToken ;
        temp->mToken->mToken = new char[ strlen( tempC ) + 14 ] ;
        strcat( temp->mToken->mToken, "#<procedure " ) ;
        strcat( temp->mToken->mToken, tempC ) ;
        strcat( temp->mToken->mToken, ">" ) ;
       
        value = temp ;
        
        return true ;
      } // else if
      else if ( userDefined ) { // it's an user-defined function
        CorrespondingTreePtr temp = new CorrespondingTree ;
        temp->mToken = head->mToken ;
        char * tempC = temp->mToken->mToken ;
        temp->mToken->mToken = new char[ strlen( tempC ) + 14 ] ;
        strcat( temp->mToken->mToken, "#<procedure " ) ;
        strcat( temp->mToken->mToken, tempC ) ;
        strcat( temp->mToken->mToken, ">" ) ;
        
        value = temp ;
        
        return true ;
      } // else if
      else { // didn't find the binding, not an internal function , it's an unbound symbol
        Error temp ;
        temp.mErrorType = UNBOUND ;
        temp.mToken = head->mToken->mToken ;
        
        mErrorVct->push_back( temp ) ;
        
        return false ;
      } // else
      
    } // else if
    // is a main S-expression
    else {
      if ( NonList( head ) ) {
        Error temp ;
        temp.mErrorType = NONLIST ;
        temp.mBinding = head ;
        
        mErrorVct->push_back( temp ) ;
        
        return false ;
      } // if
      // first atom is not a symbol
      else if ( head->mLeftNode->mToken != NULL && head->mLeftNode->mToken->mTokenType != SYMBOL &&
                Scanner::IsAtom( head->mLeftNode->mToken->mTokenType ) ) {
        Error temp ;
        temp.mErrorType = APPLYNONFUNC ;
        temp.mToken = head->mLeftNode->mToken->mToken ;
        
        mErrorVct->push_back( temp ) ;
        
        return false ;
      } // else if
      else if ( head->mLeftNode->mToken != NULL && ( head->mLeftNode->mToken->mTokenType == SYMBOL ||
                                                     head->mLeftNode->mToken->mTokenType == QUOTE ) ) {
        bool userDefined = false ;
        bool error = false ;
        
        CorrespondingTreePtr binding = GetBindingAndEval( head->mLeftNode->mToken,
                                                          userDefined, error, level ) ;
        TokenPtr func ;
        
        if ( error ) {
          return false ;
        } // if
 
        if ( binding != NULL ) { // found binding
          // but it's a symbol
          if ( binding->mToken == NULL || binding->mToken->mTokenType != SYMBOL ) {
            Error temp ;
            temp.mErrorType = APPLYNONFUNC ;
            if ( binding->mToken == NULL ) {
              
              temp.mToken = binding->mLeftNode->mToken->mToken ;
            } // if
            else {
              
              temp.mToken = binding->mToken->mToken ;
            } // else
            
            mErrorVct->push_back( temp ) ;
            
            return false ;
          } // if
          
          func = binding->mToken ;
        } // if
        else { // didn't found binding, maybe the internal func
          
          func = head->mLeftNode->mToken ;
        } // else
        
        if ( userDefined ) { // didn't done yet!!!!!
          CorrespondingTreePtr temp = new CorrespondingTree ;
          temp->mToken = head->mLeftNode->mToken ;
          char * tempC = temp->mToken->mToken ;
          temp->mToken->mToken = new char[ strlen( tempC ) + 14 ] ;
          strcat( temp->mToken->mToken, "#<procedure " ) ;
          strcat( temp->mToken->mToken, tempC ) ;
          strcat( temp->mToken->mToken, ">" ) ;
          
          value = temp ;
          
          return true ;
        } // if
        else if ( IsPrimitiveFunc( func ) ) {
          if ( level != 0 && ( strcmp( func->mToken, "clean-environment" ) == 0 ||
                               strcmp( func->mToken, "define" ) == 0 ||
                               strcmp( func->mToken, "exit" ) == 0 ) ) {
            Error temp ;
            temp.mErrorType = LEVEL ;
            temp.mToken = func->mToken ;
            
            mErrorVct->push_back( temp ) ;
            
            return false ;
          } // if
          else if ( strcmp( func->mToken, "define" ) == 0 )  {
            if ( CheckFormat( func, head ) ) { // right format
              
              return Define( head->mRightNode->mLeftNode,
                             head->mRightNode->mRightNode->mLeftNode ) ;
            } // if
            else { // wrong format
              value = NULL ;
              
              return false ;
            } // else
            
          } // else if
          else if ( strcmp( func->mToken, "if" ) == 0 )  {
            bool error = false ;
            value = CommandIf( head, level, error ) ;
            if ( ! error ) {
              
              return true ;
            } // if
            else {
              value = NULL ;
              
              return false ;
            } // else
            
          } // else if
          else if ( strcmp( func->mToken, "cond" ) == 0 )  {
            bool error = false ;
            value = CommandCond( head, level, error ) ;
            if ( error ) {
              value = NULL ;
              
              return false ;
            } // if
            else {
              
              return true ;
            } // else
            
          } // else if
          else if ( strcmp( func->mToken, "and" ) == 0 )  {
            if ( CheckFormat( func, head ) ) { // right format
              
              return CommandAnd( head->mRightNode, value, level ) ;
            } // if
            else { // wrong format
              value = NULL ;
              
              return false ;
            } // else
            
          } // else if
          else if ( strcmp( func->mToken, "or" ) == 0 )  {
            if ( CheckFormat( func, head ) ) { // right format
              
              return CommandOr( head->mRightNode, value, level ) ;
            } // if
            else { // wrong format
              value = NULL ;
              
              return false ;
            } // else
            
          } // else if
          else if ( strcmp( func->mToken, "quote" ) == 0 ||
                    strcmp( func->mToken, "\'" ) == 0 )  {
            if ( CheckFormat( func, head ) ) { // right format
              value = head->mRightNode->mLeftNode ;
              
              return true ;
            } // if
            else { // wrong format
              value = NULL ;
              
              return false ;
            } // else
            
          } // else if
          else if ( strcmp( func->mToken, "eqv?" ) == 0 )  {
            if ( CheckFormat( func, head ) ) { // right format
              bool error = false ;
              value = CommandEqv( head->mRightNode->mLeftNode,
                                  head->mRightNode->mRightNode->mLeftNode,
                                  level, error ) ;
              if ( error ) {
                value = NULL ;
                
                return false ;
              } // if
              else {
                
                return  true ;
              } // else
              
            } // if
            else { // wrong format
              value = NULL ;
              
              return false ;
            } // else
            
          } // else if
          else { // func is a known function
            if ( CheckFormat( func, head ) ) { // right format
              // proceed
              if ( EvalArgs( head->mRightNode, level ) ) {
                if ( strcmp( func->mToken, "clean-environment" ) == 0 ) {
                  cout << "environment cleaned" << endl ;
                  CleanEnvironment() ;
                } // if
                else if ( strcmp( func->mToken, "exit" ) == 0 )  {
                  ;
                } // else if
                else if ( strcmp( func->mToken, "cons" ) == 0 )  {
                  
                  value = Cons( head->mRightNode->mLeftNode,
                                head->mRightNode->mRightNode->mLeftNode ) ;
                } // else if
                else if ( strcmp( func->mToken, "list" ) == 0 )  {
                  
                  value = CommandList( head->mRightNode ) ;
                } // else if
                else if ( strcmp( func->mToken, "car" ) == 0 )  {
                  
                  value = CommandCar( head->mRightNode ) ;
                } // else if
                else if ( strcmp( func->mToken, "cdr" ) == 0 )  {
                  
                  value = CommandCdr( head->mRightNode ) ;
                } // else if
                else if ( strcmp( func->mToken, "atom?" ) == 0 )  {
                  
                  value = CommandIsAtom( head->mRightNode ) ;
                } // else if
                else if ( strcmp( func->mToken, "pair?" ) == 0 )  {
                  
                  value = CommandIsPair( head->mRightNode ) ;
                } // else if
                else if ( strcmp( func->mToken, "list?" ) == 0 )  {
                  
                  value = CommandIsList( head->mRightNode ) ;
                } // else if
                else if ( strcmp( func->mToken, "null?" ) == 0 )  {
                  
                  value = CommandIsNull( head->mRightNode ) ;
                } // else if
                else if ( strcmp( func->mToken, "real?" ) == 0 ||
                          strcmp( func->mToken, "number?" ) == 0 )  {
                  
                  value = CommandIsNumber( head->mRightNode ) ;
                } // else if
                else if ( strcmp( func->mToken, "integer?" ) == 0 )  {
                  
                  value = CommandIsInteger( head->mRightNode ) ;
                } // else if
                else if ( strcmp( func->mToken, "string?" ) == 0 )  {
                  
                  value = CommandIsString( head->mRightNode ) ;
                } // else if
                else if ( strcmp( func->mToken, "boolean?" ) == 0 )  {
                  
                  value = CommandIsBoolean( head->mRightNode ) ;
                } // else if
                else if ( strcmp( func->mToken, "symbol?" ) == 0 )  {
                  
                  value = CommandIsSymbol( head->mRightNode ) ;
                } // else if
                else if ( strcmp( func->mToken, "+" ) == 0 )  {
                  bool hasFloat = false ;
                  if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat, func ) ) {
                    value = CommandPlus( head->mRightNode, hasFloat ) ;
                    
                    return true ;
                  } // if
                  else {
                    value = NULL ;
                    
                    return false ;
                  } // else
                  
                } // else if
                else if ( strcmp( func->mToken, "-" ) == 0 )  {
                  bool hasFloat = false ;
                  if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat, func ) ) {
                    value = CommandMinus( head->mRightNode, hasFloat ) ;
                    
                    return true ;
                  } // if
                  else {
                    value = NULL ;
                    
                    return false ;
                  } // else
                  
                } // else if
                else if ( strcmp( func->mToken, "*" ) == 0 )  {
                  bool hasFloat = false ;
                  if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat, func ) ) {
                    value = CommandMulti( head->mRightNode, hasFloat ) ;
                    
                    return true ;
                  } // if
                  else {
                    value = NULL ;
                    
                    return false ;
                  } // else
                  
                } // else if
                else if ( strcmp( func->mToken, "/" ) == 0 )  {
                  bool hasFloat = false ;
                  if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat, func ) ) {
                    bool error = false ;
                    value = CommandDivide( head->mRightNode, hasFloat, error ) ;
                    if ( error ) {
                      value = NULL ;
                      
                      return false ;
                    } // if
                    else {
                      
                      return true ;
                    } // else
                    
                  } // if
                  else {
                    value = NULL ;
                    
                    return false ;
                  } // else
                  
                } // else if
                else if ( strcmp( func->mToken, "not" ) == 0 )  {
                  value = CommandNot( head->mRightNode ) ;
                  
                  return true ;
                } // else if
                else if ( strcmp( func->mToken, ">" ) == 0 )  {
                  bool hasFloat = false ;
                  if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat, func ) ) {
                    if ( CommandBiggerNum( head->mRightNode ) ) {
                      value = new CorrespondingTree ;
                      value->mToken = new Token ;
                      value->mToken->mTokenType = T ;
                      
                    } // if
                    else {
                      value = new CorrespondingTree ;
                      value->mToken = new Token ;
                      value->mToken->mTokenType = NIL ;
                      
                    } // else
                    
                    return true ;
                  } // if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat ) )
                  else {
                    value = NULL ;
                    
                    return false ;
                  } // else
                  
                } // else if
                else if ( strcmp( func->mToken, ">=" ) == 0 )  {
                  bool hasFloat = false ;
                  if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat, func ) ) {
                    if ( CommandBiggerEqvNum( head->mRightNode ) ) {
                      value = new CorrespondingTree ;
                      value->mToken = new Token ;
                      value->mToken->mTokenType = T ;
                      
                    } // if
                    else {
                      value = new CorrespondingTree ;
                      value->mToken = new Token ;
                      value->mToken->mTokenType = NIL ;
                      
                    } // else
                    
                    return true ;
                  } // if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat ) )
                  else {
                    value = NULL ;
                    
                    return false ;
                  } // else
                  
                } // else if
                else if ( strcmp( func->mToken, "<" ) == 0 )  {
                  bool hasFloat = false ;
                  if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat, func ) ) {
                    if ( CommandSmallerNum( head->mRightNode ) ) {
                      value = new CorrespondingTree ;
                      value->mToken = new Token ;
                      value->mToken->mTokenType = T ;
                      
                    } // if
                    else {
                      value = new CorrespondingTree ;
                      value->mToken = new Token ;
                      value->mToken->mTokenType = NIL ;
                      
                    } // else
                    
                    return true ;
                  } // if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat ) )
                  else {
                    value = NULL ;
                    
                    return false ;
                  } // else
                  
                } // else if
                else if ( strcmp( func->mToken, "<=" ) == 0 )  {
                  bool hasFloat = false ;
                  if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat, func ) ) {
                    if ( CommandSmallerEqvNum( head->mRightNode ) ) {
                      value = new CorrespondingTree ;
                      value->mToken = new Token ;
                      value->mToken->mTokenType = T ;
                      
                    } // if
                    else {
                      value = new CorrespondingTree ;
                      value->mToken = new Token ;
                      value->mToken->mTokenType = NIL ;
                      
                    } // else
                    
                    return true ;
                  } // if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat ) )
                  else {
                    value = NULL ;
                    
                    return false ;
                  } // else
                  
                } // else if
                else if ( strcmp( func->mToken, "=" ) == 0 )  {
                  bool hasFloat = false ;
                  if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat, func ) ) {
                    if ( CommandEqvNum( head->mRightNode ) ) {
                      value = new CorrespondingTree ;
                      value->mToken = new Token ;
                      value->mToken->mTokenType = T ;
                      
                    } // if
                    else {
                      value = new CorrespondingTree ;
                      value->mToken = new Token ;
                      value->mToken->mTokenType = NIL ;
                      
                    } // else
                    
                    return true ;
                  } // if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat ) )
                  else {
                    value = NULL ;
                    
                    return false ;
                  } // else
                  
                } // else if
                else if ( strcmp( func->mToken, "string-append" ) == 0 )  {
                  if ( CheckArgsTypeIsString( head->mRightNode, func ) ) {
                    value = CommandStringAppend( head->mRightNode ) ;
                    
                    return true ;
                  } // if ( CheckArgsTypeIsString( head->mRightNode ) )
                  else {
                    
                    return false ;
                  } // else
                  
                } // else if
                else if ( strcmp( func->mToken, "string>?" ) == 0 )  {
                  if ( CheckArgsTypeIsString( head->mRightNode, func ) ) {
                    if ( CommandBiggerString( head->mRightNode ) ) {
                      value = new CorrespondingTree ;
                      value->mToken = new Token ;
                      value->mToken->mTokenType = T ;
                      
                    } // if
                    else {
                      value = new CorrespondingTree ;
                      value->mToken = new Token ;
                      value->mToken->mTokenType = NIL ;
                      
                    } // else
                    
                    return true ;
                  } // if
                  else {
                    value = NULL ;
                    
                    return false ;
                  } // else
                  
                } // else if
                else if ( strcmp( func->mToken, "string<?" ) == 0 )  {
                  if ( CheckArgsTypeIsString( head->mRightNode, func ) ) {
                    if ( CommandSmallerString( head->mRightNode ) ) {
                      value = new CorrespondingTree ;
                      value->mToken = new Token ;
                      value->mToken->mTokenType = T ;
                      
                    } // if
                    else {
                      value = new CorrespondingTree ;
                      value->mToken = new Token ;
                      value->mToken->mTokenType = NIL ;
                      
                    } // else
                    
                    return true ;
                  } // if
                  else {
                    value = NULL ;
                    
                    return false ;
                  } // else
                  
                } // else if
                else if ( strcmp( func->mToken, "string=?" ) == 0 )  {
                  if ( CheckArgsTypeIsString( head->mRightNode, func ) ) {
                    if ( CommandEqvString( head->mRightNode ) ) {
                      value = new CorrespondingTree ;
                      value->mToken = new Token ;
                      value->mToken->mTokenType = T ;
                      
                    } // if
                    else {
                      value = new CorrespondingTree ;
                      value->mToken = new Token ;
                      value->mToken->mTokenType = NIL ;
                      
                    } // else
                    
                    return true ;
                  } // if
                  else {
                    value = NULL ;
                    
                    return false ;
                  } // else
                  
                } // else if
                else if ( strcmp( func->mToken, "equal?" ) == 0 )  {
                  bool error = false ;
                  value = CommandEqual( head->mRightNode->mLeftNode,
                                        head->mRightNode->mRightNode->mLeftNode,
                                        level, error ) ;
                  if ( error ) {
                    value = NULL ;
                    
                    return false ;
                  } // if
                  else {
                    
                    return  true ;
                  } // else
                  
                } // else if
                else if ( strcmp( func->mToken, "begin" ) == 0 )  {
                  bool error = false ;
                  value = CommandBegin( head->mRightNode, level, error ) ;
                  if ( ! error ) {
                    
                    return true ;
                  } // if
                  else {
                    value = NULL ;
                    
                    return false ;
                  } // else
                  
                } // else if
                else {
                  cout << "why are you here ? QQ" << endl ;
                  
                  return false ;
                } // else
                
              } // if ( EvalArgs( head, level ) )
              else {
                
                return false ;
              } // else
              
              return true ;
            } // if
            else { // wrong format
              value = NULL ;
              
              return false ;
            } // else
            
          } // else
          
        } // if ( IsPrimitiveFunc( func ) )
        else { // not a known function, might not a symbol, or a unbound symbol
          if ( head->mLeftNode->mToken == func ) {
            Error temp ;
            temp.mErrorType = UNBOUND ;
            temp.mToken = head->mLeftNode->mToken->mToken ;
            
            mErrorVct->push_back( temp ) ;
            
            return false ;
          } // if ( head->mLeftNode->mToken->mTokenType == SYMBOL )
          else { // not a symbol
            Error temp ;
            temp.mErrorType = NONFUNCTION ;
            temp.mBinding = binding ;
            
            mErrorVct->push_back( temp ) ;
            
            return false ;
          } // else
          
        } // else
        
      } // else if ( head->mLeftNode->mToken != NULL && head->mLeftNode->mToken->mTokenType == SYMBOL )
      else { // the first argument is a main s-Expression
        if ( Eval( head->mLeftNode, value, level + 1 ) ) {
          if ( value->mToken != NULL && IsPrimitiveFunc( value->mToken ) ) {
            TokenPtr func = value->mToken ;
            if ( level != 0 && ( strcmp( func->mToken, "clean-environment" ) == 0 ||
                                 strcmp( func->mToken, "define" ) == 0 ||
                                 strcmp( func->mToken, "exit" ) == 0 ) ) {
              Error temp ;
              temp.mErrorType = LEVEL ;
              temp.mToken = func->mToken ;
              
              mErrorVct->push_back( temp ) ;
              
              return false ;
            } // if
            else if ( strcmp( func->mToken, "define" ) == 0 )  {
              if ( CheckFormat( func, head ) ) { // right format
                
                return Define( head->mRightNode->mLeftNode,
                               head->mRightNode->mRightNode->mLeftNode ) ;
              } // if
              else { // wrong format
                value = NULL ;
                
                return false ;
              } // else
              
            } // else if
            else if ( strcmp( func->mToken, "if" ) == 0 )  {
              bool error = false ;
              value = CommandIf( head, level, error ) ;
              if ( ! error ) {
                
                return true ;
              } // if
              else {
                value = NULL ;
                
                return false ;
              } // else
              
            } // else if
            else if ( strcmp( func->mToken, "cond" ) == 0 )  {
              bool error = false ;
              value = CommandCond( head, level, error ) ;
              if ( error ) {
                value = NULL ;
                
                return false ;
              } // if
              else {
                
                return true ;
              } // else
              
            } // else if
            else if ( strcmp( func->mToken, "and" ) == 0 )  {
              if ( CheckFormat( func, head ) ) { // right format
                
                return CommandAnd( head->mRightNode, value, level ) ;
              } // if
              else { // wrong format
                value = NULL ;
                
                return false ;
              } // else
              
            } // else if
            else if ( strcmp( func->mToken, "or" ) == 0 )  {
              if ( CheckFormat( func, head ) ) { // right format
                
                return CommandOr( head->mRightNode, value, level ) ;
              } // if
              else { // wrong format
                value = NULL ;
                
                return false ;
              } // else
              
            } // else if
            else if ( strcmp( func->mToken, "quote" ) == 0 ||
                      strcmp( func->mToken, "\'" ) == 0 )  {
              if ( CheckFormat( func, head ) ) { // right format
                value = head->mRightNode->mLeftNode ;
                
                return true ;
              } // if
              else { // wrong format
                value = NULL ;
                
                return false ;
              } // else
              
            } // else if
            else if ( strcmp( func->mToken, "eqv?" ) == 0 )  {
              if ( CheckFormat( func, head ) ) { // right format
                bool error = false ;
                value = CommandEqv( head->mRightNode->mLeftNode,
                                    head->mRightNode->mRightNode->mLeftNode,
                                    level, error ) ;
                if ( error ) {
                  value = NULL ;
                  
                  return false ;
                } // if
                else {
                  
                  return  true ;
                } // else
                
              } // if
              else { // wrong format
                value = NULL ;
                
                return false ;
              } // else
              
            } // else if
            else { // func is a known function
              if ( CheckFormat( func, head ) ) { // right format
                // proceed
                if ( EvalArgs( head->mRightNode, level ) ) {
                  if ( strcmp( func->mToken, "clean-environment" ) == 0 ) {
                    CleanEnvironment() ;
                    cout << "environment cleaned" << endl ;
                    
                  } // if
                  else if ( strcmp( func->mToken, "exit" ) == 0 )  {
                    ;
                  } // else if
                  else if ( strcmp( func->mToken, "cons" ) == 0 )  {
                    
                    value = Cons( head->mRightNode->mLeftNode,
                                  head->mRightNode->mRightNode->mLeftNode ) ;
                  } // else if
                  else if ( strcmp( func->mToken, "list" ) == 0 )  {
                    
                    value = CommandList( head->mRightNode ) ;
                  } // else if
                  else if ( strcmp( func->mToken, "car" ) == 0 )  {
                    
                    value = CommandCar( head->mRightNode ) ;
                  } // else if
                  else if ( strcmp( func->mToken, "cdr" ) == 0 )  {
                    
                    value = CommandCdr( head->mRightNode ) ;
                  } // else if
                  else if ( strcmp( func->mToken, "atom?" ) == 0 )  {
                    
                    value = CommandIsAtom( head->mRightNode ) ;
                  } // else if
                  else if ( strcmp( func->mToken, "pair?" ) == 0 )  {
                    
                    value = CommandIsPair( head->mRightNode ) ;
                  } // else if
                  else if ( strcmp( func->mToken, "list?" ) == 0 )  {
                    
                    value = CommandIsList( head->mRightNode ) ;
                  } // else if
                  else if ( strcmp( func->mToken, "null?" ) == 0 )  {
                    
                    value = CommandIsNull( head->mRightNode ) ;
                  } // else if
                  else if ( strcmp( func->mToken, "real?" ) == 0 ||
                            strcmp( func->mToken, "number?" ) == 0 )  {
                    
                    value = CommandIsNumber( head->mRightNode ) ;
                  } // else if
                  else if ( strcmp( func->mToken, "integer?" ) == 0 )  {
                    
                    value = CommandIsInteger( head->mRightNode ) ;
                  } // else if
                  else if ( strcmp( func->mToken, "string?" ) == 0 )  {
                    
                    value = CommandIsString( head->mRightNode ) ;
                  } // else if
                  else if ( strcmp( func->mToken, "boolean?" ) == 0 )  {
                    
                    value = CommandIsBoolean( head->mRightNode ) ;
                  } // else if
                  else if ( strcmp( func->mToken, "symbol?" ) == 0 )  {
                    
                    value = CommandIsSymbol( head->mRightNode ) ;
                  } // else if
                  else if ( strcmp( func->mToken, "+" ) == 0 )  {
                    bool hasFloat = false ;
                    if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat, func ) ) {
                      value = CommandPlus( head->mRightNode, hasFloat ) ;
                      
                      return true ;
                    } // if
                    else {
                      value = NULL ;
                      
                      return false ;
                    } // else
                    
                  } // else if
                  else if ( strcmp( func->mToken, "-" ) == 0 )  {
                    bool hasFloat = false ;
                    if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat, func ) ) {
                      value = CommandMinus( head->mRightNode, hasFloat ) ;
                      
                      return true ;
                    } // if
                    else {
                      value = NULL ;
                      
                      return false ;
                    } // else
                    
                  } // else if
                  else if ( strcmp( func->mToken, "*" ) == 0 )  {
                    bool hasFloat = false ;
                    if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat, func ) ) {
                      value = CommandMulti( head->mRightNode, hasFloat ) ;
                      
                      return true ;
                    } // if
                    else {
                      value = NULL ;
                      
                      return false ;
                    } // else
                    
                  } // else if
                  else if ( strcmp( func->mToken, "/" ) == 0 )  {
                    bool hasFloat = false ;
                    if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat, func ) ) {
                      bool error = false ;
                      value = CommandDivide( head->mRightNode, hasFloat, error ) ;
                      if ( error ) {
                        value = NULL ;
                        
                        return false ;
                      } // if
                      else {
                        
                        return true ;
                      } // else
                      
                    } // if
                    else {
                      value = NULL ;
                      
                      return false ;
                    } // else
                    
                  } // else if
                  else if ( strcmp( func->mToken, "not" ) == 0 )  {
                    value = CommandNot( head->mRightNode ) ;
                    
                    return true ;
                  } // else if
                  else if ( strcmp( func->mToken, ">" ) == 0 )  {
                    bool hasFloat = false ;
                    if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat, func ) ) {
                      if ( CommandBiggerNum( head->mRightNode ) ) {
                        value = new CorrespondingTree ;
                        value->mToken = new Token ;
                        value->mToken->mTokenType = T ;
                        
                      } // if
                      else {
                        value = new CorrespondingTree ;
                        value->mToken = new Token ;
                        value->mToken->mTokenType = NIL ;
                        
                      } // else
                      
                      return true ;
                    } // if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat ) )
                    else {
                      value = NULL ;
                      
                      return false ;
                    } // else
                    
                  } // else if
                  else if ( strcmp( func->mToken, ">=" ) == 0 )  {
                    bool hasFloat = false ;
                    if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat, func ) ) {
                      if ( CommandBiggerEqvNum( head->mRightNode ) ) {
                        value = new CorrespondingTree ;
                        value->mToken = new Token ;
                        value->mToken->mTokenType = T ;
                        
                      } // if
                      else {
                        value = new CorrespondingTree ;
                        value->mToken = new Token ;
                        value->mToken->mTokenType = NIL ;
                        
                      } // else
                      
                      return true ;
                    } // if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat ) )
                    else {
                      value = NULL ;
                      
                      return false ;
                    } // else
                    
                  } // else if
                  else if ( strcmp( func->mToken, "<" ) == 0 )  {
                    bool hasFloat = false ;
                    if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat, func ) ) {
                      if ( CommandSmallerNum( head->mRightNode ) ) {
                        value = new CorrespondingTree ;
                        value->mToken = new Token ;
                        value->mToken->mTokenType = T ;
                        
                      } // if
                      else {
                        value = new CorrespondingTree ;
                        value->mToken = new Token ;
                        value->mToken->mTokenType = NIL ;
                        
                      } // else
                      
                      return true ;
                    } // if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat ) )
                    else {
                      value = NULL ;
                      
                      return false ;
                    } // else
                    
                  } // else if
                  else if ( strcmp( func->mToken, "<=" ) == 0 )  {
                    bool hasFloat = false ;
                    if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat, func ) ) {
                      if ( CommandSmallerEqvNum( head->mRightNode ) ) {
                        value = new CorrespondingTree ;
                        value->mToken = new Token ;
                        value->mToken->mTokenType = T ;
                        
                      } // if
                      else {
                        value = new CorrespondingTree ;
                        value->mToken = new Token ;
                        value->mToken->mTokenType = NIL ;
                        
                      } // else
                      
                      return true ;
                    } // if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat ) )
                    else {
                      value = NULL ;
                      
                      return false ;
                    } // else
                    
                  } // else if
                  else if ( strcmp( func->mToken, "=" ) == 0 )  {
                    bool hasFloat = false ;
                    if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat, func ) ) {
                      if ( CommandEqvNum( head->mRightNode ) ) {
                        value = new CorrespondingTree ;
                        value->mToken = new Token ;
                        value->mToken->mTokenType = T ;
                        
                      } // if
                      else {
                        value = new CorrespondingTree ;
                        value->mToken = new Token ;
                        value->mToken->mTokenType = NIL ;
                        
                      } // else
                      
                      return true ;
                    } // if ( CheckArgsTypeIsNumber( head->mRightNode, hasFloat ) )
                    else {
                      value = NULL ;
                      
                      return false ;
                    } // else
                    
                  } // else if
                  else if ( strcmp( func->mToken, "string-append" ) == 0 )  {
                    if ( CheckArgsTypeIsString( head->mRightNode, func ) ) {
                      value = CommandStringAppend( head->mRightNode ) ;
                      
                      return true ;
                    } // if ( CheckArgsTypeIsString( head->mRightNode ) )
                    else {
                      
                      return false ;
                    } // else
                    
                  } // else if
                  else if ( strcmp( func->mToken, "string>?" ) == 0 )  {
                    if ( CheckArgsTypeIsString( head->mRightNode, func ) ) {
                      if ( CommandBiggerString( head->mRightNode ) ) {
                        value = new CorrespondingTree ;
                        value->mToken = new Token ;
                        value->mToken->mTokenType = T ;
                        
                      } // if
                      else {
                        value = new CorrespondingTree ;
                        value->mToken = new Token ;
                        value->mToken->mTokenType = NIL ;
                        
                      } // else
                      
                      return true ;
                    } // if
                    else {
                      value = NULL ;
                      
                      return false ;
                    } // else
                    
                  } // else if
                  else if ( strcmp( func->mToken, "string<?" ) == 0 )  {
                    if ( CheckArgsTypeIsString( head->mRightNode, func ) ) {
                      if ( CommandSmallerString( head->mRightNode ) ) {
                        value = new CorrespondingTree ;
                        value->mToken = new Token ;
                        value->mToken->mTokenType = T ;
                        
                      } // if
                      else {
                        value = new CorrespondingTree ;
                        value->mToken = new Token ;
                        value->mToken->mTokenType = NIL ;
                        
                      } // else
                      
                      return true ;
                    } // if
                    else {
                      value = NULL ;
                      
                      return false ;
                    } // else
                    
                  } // else if
                  else if ( strcmp( func->mToken, "string=?" ) == 0 )  {
                    if ( CheckArgsTypeIsString( head->mRightNode, func ) ) {
                      if ( CommandEqvString( head->mRightNode ) ) {
                        value = new CorrespondingTree ;
                        value->mToken = new Token ;
                        value->mToken->mTokenType = T ;
                        
                      } // if
                      else {
                        value = new CorrespondingTree ;
                        value->mToken = new Token ;
                        value->mToken->mTokenType = NIL ;
                        
                      } // else
                      
                      return true ;
                    } // if
                    else {
                      value = NULL ;
                      
                      return false ;
                    } // else
                    
                  } // else if
                  else if ( strcmp( func->mToken, "equal?" ) == 0 )  {
                    bool error = false ;
                    value = CommandEqual( head->mRightNode->mLeftNode,
                                          head->mRightNode->mRightNode->mLeftNode,
                                          level, error ) ;
                    if ( error ) {
                      value = NULL ;
                      
                      return false ;
                    } // if
                    else {
                      
                      return  true ;
                    } // else
                    
                  } // else if
                  else if ( strcmp( func->mToken, "begin" ) == 0 )  {
                    bool error = false ;
                    value = CommandBegin( head->mRightNode, level, error ) ;
                    if ( ! error ) {
                      
                      return true ;
                    } // if
                    else {
                      value = NULL ;
                      
                      return false ;
                    } // else
                    
                  } // else if
                  else {
                    cout << "why are you here ? QQ" << endl ;
                    
                    return false ;
                  } // else
                  
                } // if ( EvalArgs( head, level ) )
                else {
                  
                  return false ;
                } // else
                
                return true ;
              } // if
              else { // wrong format
                value = NULL ;
                
                return false ;
              } // else
              
            } // else
            
          } // if ( value->mToken != NULL && IsPrimitiveFunc( value->mToken ) )
          else {
            Error temp ;
            temp.mErrorType = NONFUNCTION ;
            temp.mBinding = value ;
            
            mErrorVct->push_back( temp ) ;
            
            return false ;
          } // else
          
        } // if ( Eval( head->mLeftNode, value, level + 1 ) )
        else { // has Error when Eval()
          
          return false ;
        } // else
        
      } // else
      
    } // else
    
  } // Eval()
  // down
  
  bool EvalArgs( CorrespondingTreePtr head, int level ) {
    CorrespondingTreePtr value = NULL ;
    if ( head->mToken != NULL && head->mToken->mTokenType == NIL ) {
      
      return true ;
    } // if
    
    if ( Eval( head->mLeftNode, value, level + 1 ) ) {
      head->mLeftNode = value ;
      value = NULL ;
      if ( head->mRightNode->mRightNode != NULL ) {
        
        return EvalArgs( head->mRightNode, level ) ;
      } // if
      else {
        if ( Eval( head->mRightNode, value, level + 1 ) ) {
          head->mRightNode = value ;
          
          return  true ;
        } // if
        else {
          
          return false ;
        } // else
        
      } // else
      
    } // if ( Eval( head->mLeftNode, value, level + 1 ) )
    else {
      
      return false ;
    } // else
    
  } // EvalArgs()
  
  bool CheckFormat( TokenPtr func, CorrespondingTreePtr head ) {
    // check --"define"
    if ( strcmp( func->mToken, "define" ) == 0 ) {
      if ( head->mRightNode->mRightNode->mRightNode->mToken->mTokenType == NIL &&
           head->mLeftNode != NULL &&
           ( ( head->mLeftNode->mToken != NULL &&
               head->mLeftNode->mToken->mTokenType == SYMBOL ) ||
             ( head->mLeftNode->mToken == NULL &&
               head->mLeftNode->mLeftNode != NULL &&
               head->mLeftNode->mLeftNode->mToken != NULL &&
               head->mLeftNode->mLeftNode->mToken->mTokenType == SYMBOL ) ) ) { // right format
        
        return ! HasPrimitiveFunc( head->mRightNode->mLeftNode ) ;
      } // if
      else { // wrong format
        Error temp ;
        temp.mErrorType = FORMAT ;
        temp.mToken = head->mLeftNode->mToken->mToken ;
        temp.mBinding = head ;
        
        mErrorVct->push_back( temp ) ;
        
        return false ;
      } // else
      
    } // if
    // check --"cond"
    else if ( strcmp( func->mToken, "cond" ) == 0 ) {
      CorrespondingTreePtr walk = head->mRightNode ;
      while ( walk->mRightNode->mRightNode != NULL && walk->mLeftNode->mToken == NULL ) {
        walk = walk->mRightNode ;
      } // while
      
      if ( walk->mRightNode->mRightNode != NULL ) {
        Error temp ;
        temp.mErrorType = FORMAT ;
        temp.mToken = func->mToken ;
        temp.mBinding = head ;
        
        mErrorVct->push_back( temp ) ;
        
        return false ;
      } // if
      else {
        
        return true ;
      } // else
      
    } // else if
    // check --"if"
    // have args 2, 3
    else if ( strcmp( func->mToken, "if" ) == 0 ) {
      if ( head->mRightNode != NULL && head->mRightNode->mRightNode != NULL &&
           head->mRightNode->mRightNode->mRightNode != NULL ) {
        if ( ( head->mRightNode->mRightNode->mRightNode->mToken != NULL &&
               head->mRightNode->mRightNode->mRightNode->mToken->mTokenType == NIL ) ||
             ( head->mRightNode->mRightNode->mRightNode->mRightNode != NULL &&
               head->mRightNode->mRightNode->mRightNode->mRightNode->mToken != NULL &&
               head->mRightNode->mRightNode->mRightNode->mRightNode->mToken->mTokenType == NIL ) ) {
          return true ;
        } // if
        else {
          Error temp ;
          temp.mErrorType = ARGUMENTS ;
          temp.mToken = func->mToken ;
          
          mErrorVct->push_back( temp ) ;
          
          return false ;
        } // else
          
      } // if
      else {
        Error temp ;
        temp.mErrorType = ARGUMENTS ;
        temp.mToken = func->mToken ;
        
        mErrorVct->push_back( temp ) ;
        
        return false ;
      } // else
      
    } // else if
    // check --"and", "or"
    // have ards >= 2
    else if ( strcmp( func->mToken, "and" ) == 0 ||
              strcmp( func->mToken, "or" ) == 0 ) {
      if ( head->mRightNode != NULL && head->mRightNode->mRightNode != NULL &&
           head->mRightNode->mRightNode->mRightNode != NULL ) {
        return true ;
      } // if
      else {
        Error temp ;
        temp.mErrorType = ARGUMENTS ;
        temp.mToken = func->mToken ;
        
        mErrorVct->push_back( temp ) ;
        
        return false ;
      } // else
      
    } // else if
    // check --"quote", "\'", "car", "cdr", "atom?", "pair?", "list?",
    //         "null?", "integer?", "real?", "number?", "string?",
    //         "boolean?", "symbol?", "not"
    // have arg 1
    else if ( strcmp( func->mToken, "quote" ) == 0 ||
              strcmp( func->mToken, "\'" ) == 0 ||
              strcmp( func->mToken, "car" ) == 0 ||
              strcmp( func->mToken, "cdr" ) == 0 ||
              strcmp( func->mToken, "atom?" ) == 0 ||
              strcmp( func->mToken, "pair?" ) == 0 ||
              strcmp( func->mToken, "list?" ) == 0 ||
              strcmp( func->mToken, "null?" ) == 0 ||
              strcmp( func->mToken, "integer?" ) == 0 ||
              strcmp( func->mToken, "real?" ) == 0 ||
              strcmp( func->mToken, "number?" ) == 0 ||
              strcmp( func->mToken, "string?" ) == 0 ||
              strcmp( func->mToken, "boolean?" ) == 0 ||
              strcmp( func->mToken, "not" ) == 0 ||
              strcmp( func->mToken, "symbol?" ) == 0 ) {
      
      if ( head->mRightNode != NULL && head->mRightNode->mRightNode != NULL &&
           head->mRightNode->mRightNode->mToken != NULL &&
           head->mRightNode->mRightNode->mToken->mTokenType == NIL ) {
        return true ;
      } // if
      else {
        Error temp ;
        temp.mErrorType = ARGUMENTS ;
        temp.mToken = func->mToken ;
        
        mErrorVct->push_back( temp ) ;
        
        return false ;
      } // else
      
    } // else if
    // check --"list"
    // have arg >= 0
    else if ( strcmp( func->mToken, "list" ) == 0 ) {
      
      return true ;
    } // else if
    // check --"+", "-", "*", "/", ">", ">=", "<",
    //         "<=", "=", "string-append", "string>?", "string<?", "string=?"
    // have arg >= 2
    else if ( strcmp( func->mToken, "+" ) == 0 ||
              strcmp( func->mToken, "-" ) == 0 ||
              strcmp( func->mToken, "*" ) == 0 ||
              strcmp( func->mToken, "/" ) == 0 ||
              strcmp( func->mToken, ">" ) == 0 ||
              strcmp( func->mToken, ">=" ) == 0 ||
              strcmp( func->mToken, "<" ) == 0 ||
              strcmp( func->mToken, "<=" ) == 0 ||
              strcmp( func->mToken, "=" ) == 0 ||
              strcmp( func->mToken, "string-append" ) == 0 ||
              strcmp( func->mToken, "string>?" ) == 0 ||
              strcmp( func->mToken, "string<?" ) == 0 ||
              strcmp( func->mToken, "string=?" ) == 0 ) {
      
      if ( head->mRightNode != NULL && head->mRightNode->mRightNode != NULL &&
           head->mRightNode->mRightNode->mRightNode != NULL ) {
        return true ;
      } // if
      else {
        Error temp ;
        temp.mErrorType = ARGUMENTS ;
        temp.mToken = func->mToken ;
        
        mErrorVct->push_back( temp ) ;
        
        return false ;
      } // else
      
    } // else if
    // check --"cons"
    // have arg = 2
    else if ( strcmp( func->mToken, "cons" ) == 0 ||
              strcmp( func->mToken, "eqv?" ) == 0 ||
              strcmp( func->mToken, "equal?" ) == 0) {
      if ( head->mRightNode != NULL && head->mRightNode->mRightNode != NULL &&
           head->mRightNode->mRightNode->mRightNode != NULL &&
           head->mRightNode->mRightNode->mRightNode->mToken != NULL &&
           head->mRightNode->mRightNode->mRightNode->mToken->mTokenType == NIL ) {
        return true ;
      } // if
      else {
        Error temp ;
        temp.mErrorType = ARGUMENTS ;
        temp.mToken = func->mToken ;
        
        mErrorVct->push_back( temp ) ;
        
        return false ;
      } // else
      
    } // else if ( strcmp( func->mToken, "cons" ) == 0 )
    else if ( strcmp( func->mToken, "clean-environment" ) == 0 ) {
      if ( head->mRightNode->mToken != NULL &&
           head->mRightNode->mToken->mTokenType == NIL &&
           head->mRightNode->mRightNode == NULL ) {
        
        return true ;
      } // if
      else {
        Error temp ;
        temp.mErrorType = ARGUMENTS ;
        temp.mToken = func->mToken ;
        
        mErrorVct->push_back( temp ) ;
        
        return false ;
      } // else
      
    } // else if ( strcmp( func->mToken, "clean-environment" ) == 0 )
    else {
      cout << "not defined format." << endl ;
      
      return false ;
    } // else
    
  } // CheckFormat()
  
  bool CheckArgsTypeIsNumber( CorrespondingTreePtr head, bool & hasFloat, TokenPtr func ) {
    if ( head == NULL ) {
      cout << "should not be here !(CheckArgsTypeIsNumber)" << endl ;
      
      return false ;
    } // if
    else if ( head->mRightNode == NULL &&
              head->mToken != NULL &&
              head->mToken->mTokenType == NIL ) {
      
      return true ;
    } // else if
    else if ( head->mRightNode == NULL ) {
      if ( head->mToken != NULL ) {
        if ( head->mToken->mTokenType == INT ) {
          
          return true ;
        } // if
        else if ( head->mToken->mTokenType == FLOAT ) {
          hasFloat = true ;
          
          return true ;
        } // else if ( head->mToken->mTokenType == FLOAT )
        else {
          Error temp ;
          temp.mErrorType = ARGTYPE ;
          temp.mBinding = head ;
          temp.mToken = func->mToken ;
          
          mErrorVct->push_back( temp ) ;
          
          return false ;
        } // else
        
      } // if ( head->mRightNode->mRightNode == NULL )
      else {
        cout << "should not be here !(CheckArgsTypeIsNumber)" << endl ;
        
        return false ;
      } // else
      
    } // else if
    else {
      
      return CheckArgsTypeIsNumber( head->mLeftNode, hasFloat, func ) &&
             CheckArgsTypeIsNumber( head->mRightNode, hasFloat, func ) ;
    } // else
    
  } // CheckArgsTypeIsNumber()
  
  bool CheckArgsTypeIsString( CorrespondingTreePtr head, TokenPtr func ) {
    if ( head == NULL ) {
      cout << "should not be here !(CheckArgsTypeIsString)" << endl ;
      
      return false ;
    } // if
    else if ( head->mRightNode == NULL &&
              head->mToken != NULL &&
              head->mToken->mTokenType == NIL ) {
      
      return true ;
    } // else if
    else if ( head->mRightNode == NULL ) {
      if ( head->mToken != NULL ) {
        if ( head->mToken->mTokenType == STRING ) {
          
          return true ;
        } // if
        else {
          Error temp ;
          temp.mErrorType = ARGTYPE ;
          temp.mBinding = head ;
          temp.mToken = func->mToken ;
          
          mErrorVct->push_back( temp ) ;
          
          return false ;
        } // else
        
      } // if ( head->mRightNode->mRightNode == NULL )
      else {
        cout << "should not be here !(CheckArgsTypeIsString)" << endl ;
        
        return false ;
      } // else
      
    } // else if
    else {
      
      return CheckArgsTypeIsString( head->mLeftNode, func ) &&
             CheckArgsTypeIsString( head->mRightNode, func ) ;
    } // else
    
  } // CheckArgsTypeIsString()
  
  bool AllSymbolsAreBound( CorrespondingTreePtr head ) {
    if ( head->mLeftNode != NULL && head->mLeftNode->mToken != NULL &&
        head->mLeftNode->mToken->mTokenType == QUOTE ) {
      
      return true ;
    } // if
    else if ( head->mRightNode == NULL ) {
      if ( head->mToken != NULL &&
           head->mToken->mTokenType == SYMBOL ) {
        if ( IsPrimitiveFunc( head->mToken ) ) {
          
          return true ;
        } // if
        else if ( IsBound( head->mToken ) ) {
          
          return true ;
        } // else if
        else {
          Error temp ;
          temp.mErrorType = UNBOUND ;
          temp.mToken = head->mToken->mToken ;
          
          mErrorVct->push_back( temp ) ;
          
          return false ;
        } // else
        
      } // if
      else {
        
        return true ;
      } // else
      
    } // else if
    else {
      
      return ( AllSymbolsAreBound( head->mLeftNode ) &&
               AllSymbolsAreBound( head->mRightNode ) ) ;
    } // else
    
  } // AllSymbolsAreBound()
  
  bool IsBound( TokenPtr symbol ) {
    int len = mSymbolVct->size() ;
    for ( int i = 0 ; i < len ; i++ ) {
      if ( mSymbolVct->at( i ).mSymbol->mToken != NULL &&
           ( strcmp( mSymbolVct->at( i ).mSymbol->mToken->mToken,
                     symbol->mToken ) == 0 ) ) {
        
        return true ;
      } // if
      else if ( mSymbolVct->at( i ).mSymbol->mLeftNode->mToken != NULL &&
                ( strcmp( mSymbolVct->at( i ).mSymbol->mLeftNode->mToken->mToken,
                          symbol->mToken ) == 0 ) ) {
        return true ;
      } // else if
      
    } // for
    
    return false ;
  } // IsBound()
  
  bool HasPrimitiveFunc( CorrespondingTreePtr head ) {
    if ( head == NULL ) {
      
      return false ;
    } // if
    else if ( head->mToken != NULL ) {
      
      return IsPrimitiveFunc( head->mToken ) ;
    } // else if
    else {
      
      return HasPrimitiveFunc( head->mLeftNode ) || HasPrimitiveFunc( head->mLeftNode ) ;
    } // else
    
  } // HasPrimitiveFunc()

  bool IsPrimitiveFunc( TokenPtr head ) {
    if ( head == NULL ) {
      return false ;
    } // if
    else if ( strcmp( head->mToken, "define" ) == 0 ||
              strcmp( head->mToken, "cons" ) == 0 ||
              strcmp( head->mToken, "list" ) == 0 ||
              strcmp( head->mToken, "quote" ) == 0 ||
              strcmp( head->mToken, "\'" ) == 0 ||
              strcmp( head->mToken, "car" ) == 0 ||
              strcmp( head->mToken, "cdr" ) == 0 ||
              strcmp( head->mToken, "atom?" ) == 0 ||
              strcmp( head->mToken, "pair?" ) == 0 ||
              strcmp( head->mToken, "list?" ) == 0 ||
              strcmp( head->mToken, "null?" ) == 0 ||
              strcmp( head->mToken, "real?" ) == 0 ||
              strcmp( head->mToken, "integer?" ) == 0 ||
              strcmp( head->mToken, "number?" ) == 0 ||
              strcmp( head->mToken, "string?" ) == 0 ||
              strcmp( head->mToken, "boolean?" ) == 0 ||
              strcmp( head->mToken, "symbol?" ) == 0 ||
              strcmp( head->mToken, "+" ) == 0 ||
              strcmp( head->mToken, "-" ) == 0 ||
              strcmp( head->mToken, "*" ) == 0 ||
              strcmp( head->mToken, "/" ) == 0 ||
              strcmp( head->mToken, "not" ) == 0 ||
              strcmp( head->mToken, "and" ) == 0 ||
              strcmp( head->mToken, "or" ) == 0 ||
              strcmp( head->mToken, ">" ) == 0 ||
              strcmp( head->mToken, ">=" ) == 0 ||
              strcmp( head->mToken, "<" ) == 0 ||
              strcmp( head->mToken, "<=" ) == 0 ||
              strcmp( head->mToken, "=" ) == 0 ||
              strcmp( head->mToken, "string-append" ) == 0 ||
              strcmp( head->mToken, "string>?" ) == 0 ||
              strcmp( head->mToken, "string<?" ) == 0 ||
              strcmp( head->mToken, "string=?" ) == 0 ||
              strcmp( head->mToken, "eqv?" ) == 0 ||
              strcmp( head->mToken, "equal?" ) == 0 ||
              strcmp( head->mToken, "begin" ) == 0 ||
              strcmp( head->mToken, "if" ) == 0 ||
              strcmp( head->mToken, "cond" ) == 0 ||
              strcmp( head->mToken, "clean-environment" ) == 0 ) {
      return true ;
    } // else if
    else {
      return false ;
    } // else
    
  } // IsPrimitiveFunc()
  
  bool NonList( CorrespondingTreePtr head ) {
    
    if ( head == NULL ) {
      cout << "why am I here ?" << endl ;
      
      return true ;
    } // if
    else if ( head->mLeftNode != NULL && head->mLeftNode->mToken != NULL &&
              head->mLeftNode->mToken->mTokenType == QUOTE ) {
      
      return false ;
    } // else if
    else if ( head->mRightNode == NULL ) {
      if ( head->mToken == NULL ) {
        cout << "should not be here !(NonList)" << endl ;
        
        return true ;
      } // if
      else { // if ( head->mToken != NULL )
        if ( head->mToken->mTokenType == NIL ) {
          
          return false ;
        } // if
        else {
          
          return true ;
        } // else
        
      } // else
      
    } // else if
    else { // if ( head->mRightNode != NULL )
      
      return NonList( head->mRightNode ) ;
    } // else
    
  } // NonList()
  
  bool Define( CorrespondingTreePtr symbol, CorrespondingTreePtr binding ) {
    int len = mSymbolVct->size() ;
    bool redefine = false ;
    
    if ( AllSymbolsAreBound( binding ) ) {
    
      for ( int i = 0 ; i < len && ! redefine ; i++  ) {
        if ( strcmp( mSymbolVct->at( i ).mSymbol->mToken->mToken,
                     symbol->mToken->mToken ) == 0 ) {
          mSymbolVct->at( i ).mBinding = binding ;
          redefine = true ;
        } // if
        
      } // for
      
      if ( ! redefine ) {
        Symbol temp ;
        temp.mSymbol = symbol ;
        temp.mBinding = binding ;
        
        mSymbolVct->push_back( temp ) ;
      } // if
      
      if ( symbol->mToken != NULL ) {
        cout << symbol->mToken->mToken << " defined" << endl ;
      } // if
      else {
        cout << symbol->mLeftNode->mToken->mToken << " defined" << endl ;
      } // else
      
      return true ;
    } // if ( AllSymbolsAreBound( binding ) )
    else {
      
      return false ;
    } // else
    
  } // Define()
  
  CorrespondingTreePtr GetBindingAndEval( TokenPtr symbol, bool & userDefined, bool & error, int level ) {
    int len = mSymbolVct->size() ;
    error = false ;
    
    if ( symbol != NULL ) {
      for ( int i = 0 ; i < len ; i++ ) {
        if ( mSymbolVct->at( i ).mSymbol->mToken != NULL ) {
          if ( strcmp( mSymbolVct->at( i ).mSymbol->mToken->mToken, symbol->mToken ) == 0 ) {
            CorrespondingTreePtr value ;
            if ( mSymbolVct->at( i ).mBinding->mToken != NULL &&
                 mSymbolVct->at( i ).mBinding->mToken->mTokenType == SYMBOL ) {
              value = GetBindingAndEval( mSymbolVct->at( i ).mBinding->mToken, userDefined, error, level ) ;
              
              
              return value ;
            } // if
            else {
              if ( Eval( mSymbolVct->at( i ).mBinding, value, level + 1 ) ) {
                error = false ;
                
                return value ;
              } // if
              else {
                error = true ;
                
                return NULL ;
              } // else
              
            } // else
            
          } // if
          
        } // if ( mSymbolVct->at( i ).mSymbol->mToken != NULL )
        else {
          if ( strcmp( mSymbolVct->at( i ).mSymbol->mLeftNode->mToken->mToken,
                       symbol->mToken ) == 0 ) {
            userDefined = true ;
            
            return NULL ;
          } // if
          
        } // else
        
      } // for
      
    } // if ( symbol->mToken != NUL )
    else {
      
      return NULL ;
    } // else
    
    return NULL ;
  } // GetBindingAndEval()
  
  CorrespondingTreePtr GetBinding( TokenPtr symbol ) {
    int len = mSymbolVct->size() ;
    
    if ( symbol != NULL ) {
      for ( int i = 0 ; i < len ; i++ ) {
        if ( mSymbolVct->at( i ).mSymbol->mToken != NULL ) {
          if ( strcmp( mSymbolVct->at( i ).mSymbol->mToken->mToken, symbol->mToken ) == 0 ) {
            
            if ( mSymbolVct->at( i ).mBinding->mToken != NULL &&
                 mSymbolVct->at( i ).mBinding->mToken->mTokenType == SYMBOL ) {
              
              return GetBinding( mSymbolVct->at( i ).mBinding->mToken ) ;
            } // if
            else {
              
              return mSymbolVct->at( i ).mBinding ;
            } // else
            
          } // if
          
        } // if ( mSymbolVct->at( i ).mSymbol->mToken != NULL )
        else if ( strcmp( mSymbolVct->at( i ).mSymbol->mLeftNode->mToken->mToken,
                          symbol->mToken ) == 0 ) {
            
          return mSymbolVct->at( i ).mBinding ;
        } // else if
        
      } // for
      
      return NULL ;
    } // if ( symbol->mToken != NUL )
    else {
      
      return NULL ;
    } // else
    
  } // GetBinding()
  
  CorrespondingTreePtr CommandList( CorrespondingTreePtr head ) {
    if ( head->mRightNode == NULL ) {
      
      return head ;
    } // if
    else {
      
      return Cons( head->mLeftNode, CommandList( head->mRightNode ) ) ;
    } // else
    
  } // CommandList()
  
  CorrespondingTreePtr CommandCar( CorrespondingTreePtr head ) {
    if ( head->mLeftNode->mToken != NULL ) {
      Error temp ;
      temp.mErrorType = ARGTYPE ;
      temp.mToken = head->mToken->mToken ;
      
      return NULL ;
    } // if ( head->mToken != NULL )
    else {
      
      return head->mLeftNode->mLeftNode ;
    } // else
    
  } // CommandCar()
  
  CorrespondingTreePtr CommandCdr( CorrespondingTreePtr head ) {
    if ( head->mLeftNode->mToken != NULL ) {
      Error temp ;
      temp.mErrorType = ARGTYPE ;
      temp.mToken = head->mToken->mToken ;
      
      return NULL ;
    } // if ( head->mLeftNode->mToken != NULL )
    else {
      
      return head->mLeftNode->mRightNode ;
    } // else
    
  } // CommandCdr()
  
  CorrespondingTreePtr CommandIsAtom( CorrespondingTreePtr head ) {
    if ( head->mLeftNode->mToken != NULL ) {
      if ( Scanner::IsAtom( head->mLeftNode->mToken->mTokenType ) ) {
        CorrespondingTreePtr t = new CorrespondingTree ;
        t->mToken = new Token ;
        t->mToken->mTokenType = T ;
        
        return t ;
      } // if
      else {
        CorrespondingTreePtr nil = new CorrespondingTree ;
        nil->mToken = new Token ;
        nil->mToken->mTokenType = NIL ;
        
        return nil ;
      } // else
      
    } // if ( head->mToken != NULL )
    else {
      CorrespondingTreePtr nil = new CorrespondingTree ;
      nil->mToken = new Token ;
      nil->mToken->mTokenType = NIL ;
      
      return nil ;
    } // else
    
  } // CommandIsAtom()
  
  CorrespondingTreePtr CommandIsInteger( CorrespondingTreePtr head ) {
    if ( head->mLeftNode->mToken != NULL ) {
      if ( head->mLeftNode->mToken->mTokenType == INT ) {
        CorrespondingTreePtr t = new CorrespondingTree ;
        t->mToken = new Token ;
        t->mToken->mTokenType = T ;
        
        return t ;
      } // if
      else {
        CorrespondingTreePtr nil = new CorrespondingTree ;
        nil->mToken = new Token ;
        nil->mToken->mTokenType = NIL ;
        
        return nil ;
      } // else
      
    } // if ( head->mToken != NULL )
    else {
      CorrespondingTreePtr nil = new CorrespondingTree ;
      nil->mToken = new Token ;
      nil->mToken->mTokenType = NIL ;
      
      return nil ;
    } // else
    
  } // CommandIsInteger()
  
  CorrespondingTreePtr CommandIsNumber( CorrespondingTreePtr head ) {
    if ( head->mLeftNode->mToken != NULL ) {
      if ( head->mLeftNode->mToken->mTokenType == INT ||
           head->mLeftNode->mToken->mTokenType == FLOAT ) {
        CorrespondingTreePtr t = new CorrespondingTree ;
        t->mToken = new Token ;
        t->mToken->mTokenType = T ;
        
        return t ;
      } // if
      else {
        CorrespondingTreePtr nil = new CorrespondingTree ;
        nil->mToken = new Token ;
        nil->mToken->mTokenType = NIL ;
        
        return nil ;
      } // else
      
    } // if ( head->mToken != NULL )
    else {
      CorrespondingTreePtr nil = new CorrespondingTree ;
      nil->mToken = new Token ;
      nil->mToken->mTokenType = NIL ;
      
      return nil ;
    } // else
    
  } // CommandIsNumber()
  
  CorrespondingTreePtr CommandIsString( CorrespondingTreePtr head ) {
    if ( head->mLeftNode->mToken != NULL ) {
      if ( head->mLeftNode->mToken->mTokenType == STRING ) {
        CorrespondingTreePtr t = new CorrespondingTree ;
        t->mToken = new Token ;
        t->mToken->mTokenType = T ;
        
        return t ;
      } // if
      else {
        CorrespondingTreePtr nil = new CorrespondingTree ;
        nil->mToken = new Token ;
        nil->mToken->mTokenType = NIL ;
        
        return nil ;
      } // else
      
    } // if ( head->mToken != NULL )
    else {
      CorrespondingTreePtr nil = new CorrespondingTree ;
      nil->mToken = new Token ;
      nil->mToken->mTokenType = NIL ;
      
      return nil ;
    } // else
    
  } // CommandIsString()
  
  CorrespondingTreePtr CommandIsBoolean( CorrespondingTreePtr head ) {
    if ( head->mLeftNode->mToken != NULL ) {
      if ( head->mLeftNode->mToken->mTokenType == T ||
           head->mLeftNode->mToken->mTokenType == NIL ) {
        CorrespondingTreePtr t = new CorrespondingTree ;
        t->mToken = new Token ;
        t->mToken->mTokenType = T ;
        
        return t ;
      } // if
      else {
        CorrespondingTreePtr nil = new CorrespondingTree ;
        nil->mToken = new Token ;
        nil->mToken->mTokenType = NIL ;
        
        return nil ;
      } // else
      
    } // if ( head->mToken != NULL )
    else {
      CorrespondingTreePtr nil = new CorrespondingTree ;
      nil->mToken = new Token ;
      nil->mToken->mTokenType = NIL ;
      
      return nil ;
    } // else
    
  } // CommandIsBoolean()
  
  CorrespondingTreePtr CommandIsSymbol( CorrespondingTreePtr head ) {
    if ( head->mLeftNode->mToken != NULL ) {
      if ( head->mLeftNode->mToken->mTokenType == SYMBOL ) {
        CorrespondingTreePtr t = new CorrespondingTree ;
        t->mToken = new Token ;
        t->mToken->mTokenType = T ;
        
        return t ;
      } // if
      else {
        CorrespondingTreePtr nil = new CorrespondingTree ;
        nil->mToken = new Token ;
        nil->mToken->mTokenType = NIL ;
        
        return nil ;
      } // else
      
    } // if ( head->mToken != NULL )
    else {
      CorrespondingTreePtr nil = new CorrespondingTree ;
      nil->mToken = new Token ;
      nil->mToken->mTokenType = NIL ;
      
      return nil ;
    } // else
    
  } // CommandIsSymbol()
  
  CorrespondingTreePtr CommandIsPair( CorrespondingTreePtr head ) {
    if ( head->mLeftNode->mToken != NULL ) {
      CorrespondingTreePtr nil = new CorrespondingTree ;
      nil->mToken = new Token ;
      nil->mToken->mTokenType = NIL ;
      
      return nil ;
    } // if ( head->mLeftNode->mToken != NULL )
    else {
      CorrespondingTreePtr t = new CorrespondingTree ;
      t->mToken = new Token ;
      t->mToken->mTokenType = T ;
      
      return t ;
    } // else
    
  } // CommandIsPair()
  
  CorrespondingTreePtr CommandIsNull( CorrespondingTreePtr head ) {
    if ( head->mLeftNode->mToken != NULL &&
         head->mLeftNode->mToken->mTokenType == NIL ) {
      CorrespondingTreePtr t = new CorrespondingTree ;
      t->mToken = new Token ;
      t->mToken->mTokenType = T ;
      
      return t ;
    } // if ( head->mToken != NULL )
    else {
      CorrespondingTreePtr nil = new CorrespondingTree ;
      nil->mToken = new Token ;
      nil->mToken->mTokenType = NIL ;
      
      return nil ;
    } // else
    
  } // CommandIsNull()
  
  CorrespondingTreePtr CommandIsList( CorrespondingTreePtr head ) {
    if ( head->mLeftNode->mToken != NULL ) {
      CorrespondingTreePtr nil = new CorrespondingTree ;
      nil->mToken = new Token ;
      nil->mToken->mTokenType = NIL ;
      
      return nil ;
    } // if ( head->mToken != NULL )
    else {
      CorrespondingTreePtr walk = head->mLeftNode ;
      
      while ( walk->mRightNode != NULL ) {
        walk = walk->mRightNode ;
      } // while
      
      if ( walk->mToken != NULL && walk->mToken->mTokenType == NIL ) {
        CorrespondingTreePtr t = new CorrespondingTree ;
        t->mToken = new Token ;
        t->mToken->mTokenType = T ;
        
        return t ;
      } // if ( walk->mToken != NULL && walk->mToken->mTokenType == NIL )
      else {
        CorrespondingTreePtr nil = new CorrespondingTree ;
        nil->mToken = new Token ;
        nil->mToken->mTokenType = NIL ;
        
        return nil ;
      } // else
      
    } // else
    
  } // CommandIsList()
  
  CorrespondingTreePtr CommandPlus( CorrespondingTreePtr head, bool hasFloat ) {
    CorrespondingTreePtr value = new CorrespondingTree ;
    value->mToken = new Token ;
    value->mToken->mToken = new char[20] ;
    
    if ( hasFloat ) {
      float num = PlusF( head ) ;
      sprintf( value->mToken->mToken, "%.3f", num ) ;
      value->mToken->mTokenType = FLOAT ;
      
      return value ;
    } // if ( hasFloat )
    else {
      int num = Plus( head ) ;
      sprintf( value->mToken->mToken, "%d", num ) ;
      value->mToken->mTokenType = INT ;
      
      return value ;
    } // else
    
  } // CommandPlus()
  
  int Plus( CorrespondingTreePtr head ) {
    CorrespondingTreePtr walk = head->mRightNode ;
    float value = atoi( head->mLeftNode->mToken->mToken ) ;
    
    while ( walk->mRightNode != NULL ) {
      if ( walk->mToken == NULL ) {
        
        value = value + atoi( walk->mLeftNode->mToken->mToken ) ;
      } // if
      
      walk = walk->mRightNode ;
    } // While
  
    return value ;
  } // Plus()
  
  float PlusF( CorrespondingTreePtr head ) {
    CorrespondingTreePtr walk = head->mRightNode ;
    float value = atof( head->mLeftNode->mToken->mToken ) ;
    
    while ( walk->mRightNode != NULL ) {
      if ( walk->mToken == NULL ) {
        
        value = value + atof( walk->mLeftNode->mToken->mToken ) ;
      } // if
      
      walk = walk->mRightNode ;
    } // While
  
    return value ;
  } // PlusF()
  
  CorrespondingTreePtr CommandMinus( CorrespondingTreePtr head, bool hasFloat ) {
    CorrespondingTreePtr value = new CorrespondingTree ;
    value->mToken = new Token ;
    value->mToken->mToken = new char[20] ;
    
    if ( hasFloat ) {
      float num = MinusF( head ) ;
      sprintf( value->mToken->mToken, "%.3f", num ) ;
      value->mToken->mTokenType = FLOAT ;
      
      return value ;
    } // if ( hasFloat )
    else {
      int num = Minus( head ) ;
      sprintf( value->mToken->mToken, "%d", num ) ;
      value->mToken->mTokenType = INT ;
      
      return value ;
    } // else
    
  } // CommandMinus()
  
  int Minus( CorrespondingTreePtr head ) {
    CorrespondingTreePtr walk = head->mRightNode ;
    int value = atoi( head->mLeftNode->mToken->mToken ) ;
    
    while ( walk->mRightNode != NULL ) {
      if ( walk->mToken == NULL ) {
        
        value = value - atoi( walk->mLeftNode->mToken->mToken ) ;
      } // if
      
      walk = walk->mRightNode ;
    } // While
  
    return value ;
  } // Minus()
  
  float MinusF( CorrespondingTreePtr head ) {
    CorrespondingTreePtr walk = head->mRightNode ;
    float value = atof( head->mLeftNode->mToken->mToken ) ;
    
    while ( walk->mRightNode != NULL ) {
      if ( walk->mToken == NULL ) {
        
        value = value - atof( walk->mLeftNode->mToken->mToken ) ;
      } // if
      
      walk = walk->mRightNode ;
    } // While
  
    return value ;
  } // MinusF()
  
  CorrespondingTreePtr CommandMulti( CorrespondingTreePtr head, bool hasFloat ) {
    CorrespondingTreePtr value = new CorrespondingTree ;
    value->mToken = new Token ;
    value->mToken->mToken = new char[20] ;
    
    if ( hasFloat ) {
      float num = MultiF( head ) ;
      sprintf( value->mToken->mToken, "%.3f", num ) ;
      value->mToken->mTokenType = FLOAT ;
      
      return value ;
    } // if ( hasFloat )
    else {
      int num = Multi( head ) ;
      sprintf( value->mToken->mToken, "%d", num ) ;
      value->mToken->mTokenType = INT ;
      
      return value ;
    } // else
    
  } // CommandMulti()
  
  int Multi( CorrespondingTreePtr head ) {
    CorrespondingTreePtr walk = head->mRightNode ;
    int value = atoi( head->mLeftNode->mToken->mToken ) ;
    
    while ( walk->mRightNode != NULL ) {
      if ( walk->mToken == NULL ) {
        
        value = value * atoi( walk->mLeftNode->mToken->mToken ) ;
      } // if
      
      walk = walk->mRightNode ;
    } // While
  
    return value ;
  } // Multi()
  
  float MultiF( CorrespondingTreePtr head ) {
    CorrespondingTreePtr walk = head->mRightNode ;
    float value = atof( head->mLeftNode->mToken->mToken ) ;
    
    while ( walk->mRightNode != NULL ) {
      if ( walk->mToken == NULL ) {
        
        value = value * atof( walk->mLeftNode->mToken->mToken ) ;
      } // if
      
      walk = walk->mRightNode ;
    } // While
  
    return value ;
  } // MultiF()
  
  CorrespondingTreePtr CommandDivide( CorrespondingTreePtr head, bool hasFloat, bool & error ) {
    CorrespondingTreePtr value = new CorrespondingTree ;
    value->mToken = new Token ;
    value->mToken->mToken = new char[20] ;
    
    if ( hasFloat ) {
      float num = DivideF( head, error ) ;
      sprintf( value->mToken->mToken, "%.3f", num ) ;
      value->mToken->mTokenType = FLOAT ;
      
      return value ;
    } // if ( hasFloat )
    else {
      int num = Divide( head, error ) ;
      sprintf( value->mToken->mToken, "%d", num ) ;
      value->mToken->mTokenType = INT ;
      
      return value ;
    } // else
    
  } // CommandDivide()
  
  int Divide( CorrespondingTreePtr head, bool & hasZero ) {
    CorrespondingTreePtr walk = head->mRightNode ;
    float value = atoi( head->mLeftNode->mToken->mToken ) ;
    
    while ( walk->mRightNode != NULL && ! hasZero ) {
      if ( walk->mToken == NULL ) {
        if ( strcmp( walk->mLeftNode->mToken->mToken, "0" ) == 0 ) {
          hasZero = true ;
        } // if
        else {
          value = value / atoi( walk->mLeftNode->mToken->mToken ) ;
        } // else
        
      } // if
      
      walk = walk->mRightNode ;
    } // While
    
    return value ;
  } // Divide()
  
  float DivideF( CorrespondingTreePtr head, bool & hasZero ) {
    CorrespondingTreePtr walk = head->mRightNode ;
    float value = atof( head->mLeftNode->mToken->mToken ) ;
    
    while ( walk->mRightNode != NULL ) {
      if ( walk->mToken == NULL ) {
        if ( strcmp( walk->mLeftNode->mToken->mToken, "0" ) == 0 ) {
          hasZero = true ;
        } // if
        else {
          value = value / atof( walk->mLeftNode->mToken->mToken ) ;
        } // else
        
      } // if
      
      walk = walk->mRightNode ;
    } // While
  
    return value ;
  } // DivideF()
  
  CorrespondingTreePtr CommandNot( CorrespondingTreePtr head ) {
    if ( head->mLeftNode->mToken != NULL ) {
      if ( head->mLeftNode->mToken->mTokenType == NIL ) {
        CorrespondingTreePtr t = new CorrespondingTree ;
        t->mToken = new Token ;
        t->mToken->mTokenType = T ;
        
        return t ;
      } // if ( head->mLeftNode->mToken->mTokenType == NIL )
      else {
        CorrespondingTreePtr nil = new CorrespondingTree ;
        nil->mToken = new Token ;
        nil->mToken->mTokenType = NIL ;
        
        return nil ;
      } // else
      
    } // if
    else {
      CorrespondingTreePtr nil = new CorrespondingTree ;
      nil->mToken = new Token ;
      nil->mToken->mTokenType = NIL ;
      
      return nil ;
    } // else
    
  } // CommandNot()
  
  bool CommandAnd( CorrespondingTreePtr head, CorrespondingTreePtr value, int level ) {
    if ( Eval( head->mLeftNode, value, level + 1 ) ) {
      if ( value->mToken != NULL &&
           value->mToken->mTokenType == NIL ) {
        
        return true ;
      } // if
      else if ( head->mRightNode->mRightNode == NULL ) {
        
        return true ;
      } // else if
      else {
        
        return CommandAnd( head->mRightNode, value, level ) ;
      } // else
      
    } // if ( Eval( head->mLeftNode, value, level + 1 ) )
    else {
      value = NULL ;
      
      return false ;
    } // else
    
  } // CommandAnd()
  
  bool CommandOr( CorrespondingTreePtr head, CorrespondingTreePtr value, int level ) {
    if ( Eval( head->mLeftNode, value, level + 1 ) ) {
      if ( value->mToken != NULL &&
           value->mToken->mTokenType != NIL ) {
        
        return true ;
      } // if
      else if ( head->mRightNode->mRightNode == NULL ) {
        
        return true ;
      } // else if
      else {
        
        return CommandOr( head->mRightNode, value, level ) ;
      } // else
      
    } // if ( Eval( head->mLeftNode, value, level + 1 ) )
    else {
      value = NULL ;
      
      return false ;
    } // else
    
  } // CommandOr()
  
  bool CommandBiggerNum( CorrespondingTreePtr head ) {
    if ( head->mRightNode->mRightNode == NULL ) {
      
      return true ;
    } // if
    else {
      
      return ( atof( head->mLeftNode->mToken->mToken ) >
               atof( head->mRightNode->mLeftNode->mToken->mToken ) ) &&
             CommandBiggerNum( head->mRightNode ) ;
    } // else
    
  } // CommandBiggerNum()
  
  bool CommandBiggerEqvNum( CorrespondingTreePtr head ) {
    if ( head->mRightNode->mRightNode == NULL ) {
      
      return true ;
    } // if
    else {
      
      return ( atof( head->mLeftNode->mToken->mToken ) >=
               atof( head->mRightNode->mLeftNode->mToken->mToken ) ) &&
             CommandBiggerEqvNum( head->mRightNode ) ;
    } // else
    
  } // CommandBiggerEqvNum()
  
  bool CommandSmallerNum( CorrespondingTreePtr head ) {
    if ( head->mRightNode->mRightNode == NULL ) {
      
      return true ;
    } // if
    else {
      
      return ( atof( head->mLeftNode->mToken->mToken ) <
               atof( head->mRightNode->mLeftNode->mToken->mToken ) ) &&
             CommandSmallerNum( head->mRightNode ) ;
    } // else
    
  } // CommandSmallerNum()
  
  bool CommandSmallerEqvNum( CorrespondingTreePtr head ) {
    if ( head->mRightNode->mRightNode == NULL ) {
      
      return true ;
    } // if
    else {
      
      return ( atof( head->mLeftNode->mToken->mToken ) <=
               atof( head->mRightNode->mLeftNode->mToken->mToken ) ) &&
             CommandSmallerEqvNum( head->mRightNode ) ;
    } // else
    
  } // CommandSmallerEqvNum()
  
  bool CommandEqvNum( CorrespondingTreePtr head ) {
    if ( head->mRightNode->mRightNode == NULL ) {
      
      return true ;
    } // if
    else {
      
      return ( atof( head->mLeftNode->mToken->mToken ) ==
               atof( head->mRightNode->mLeftNode->mToken->mToken ) ) &&
             CommandEqvNum( head->mRightNode ) ;
    } // else
    
  } // CommandEqvNum()
  
  CorrespondingTreePtr CommandStringAppend( CorrespondingTreePtr head ) {
    CorrespondingTreePtr walk = head ;
    CorrespondingTreePtr value = new CorrespondingTree ;
    value->mToken = new Token ;
    value->mToken->mTokenType = STRING ;
    
    CharPtr temp = NULL ;
    int len = 0 ;
    while ( walk->mRightNode != NULL ) {
      len = len + strlen( walk->mLeftNode->mToken->mToken ) ;
      temp = value->mToken->mToken ;
      value->mToken->mToken = new char[ len + 1 ] ;
      if ( temp != NULL ) {
        strcat( value->mToken->mToken, temp ) ;
        delete [] temp ;
      } // if
      
      strcat( value->mToken->mToken, walk->mLeftNode->mToken->mToken ) ;
      strcat( value->mToken->mToken, "\0" ) ;
      
      walk = walk->mRightNode ;
    } // while
    
    return value ;
  } // CommandStringAppend()
  
  bool CommandBiggerString( CorrespondingTreePtr head ) {
    if ( head->mRightNode->mRightNode == NULL ) {
      
      return true ;
    } // if
    else {
      
      return ( strcmp( head->mLeftNode->mToken->mToken,
                       head->mRightNode->mLeftNode->mToken->mToken ) > 0 ) &&
             CommandBiggerString( head->mRightNode ) ;
    } // else
    
  } // CommandBiggerString()
  
  bool CommandSmallerString( CorrespondingTreePtr head ) {
    if ( head->mRightNode->mRightNode == NULL ) {
      
      return true ;
    } // if
    else {
      
      return ( strcmp( head->mLeftNode->mToken->mToken,
                       head->mRightNode->mLeftNode->mToken->mToken ) < 0 ) &&
             CommandSmallerString( head->mRightNode ) ;
    } // else
    
  } // CommandSmallerString()
  
  bool CommandEqvString( CorrespondingTreePtr head ) {
    if ( head->mRightNode->mRightNode == NULL ) {
      
      return true ;
    } // if
    else {
      
      return ( strcmp( head->mLeftNode->mToken->mToken,
                       head->mRightNode->mLeftNode->mToken->mToken ) == 0 ) &&
             CommandSmallerString( head->mRightNode ) ;
    } // else
    
  } // CommandEqvString()
  
  CorrespondingTreePtr CommandEqv( CorrespondingTreePtr a, CorrespondingTreePtr b,
                                   int level, bool & error ) {
    if ( SameTree( a, b, true, level, error ) ) {
      CorrespondingTreePtr t = new CorrespondingTree ;
      t->mToken = new Token ;
      t->mToken->mTokenType = T ;
      
      return t ;
    } // if
    else {
      CorrespondingTreePtr nil = new CorrespondingTree ;
      nil->mToken = new Token ;
      nil->mToken->mTokenType = NIL ;
      
      return nil ;
    } // else
    
  } // CommandEqv()
  
  CorrespondingTreePtr CommandEqual( CorrespondingTreePtr a, CorrespondingTreePtr b,
                                     int level, bool & error ) {
    if ( SameTree( a, b, false, level, error ) ) {
      CorrespondingTreePtr t = new CorrespondingTree ;
      t->mToken = new Token ;
      t->mToken->mTokenType = T ;
      
      return t ;
    } // if
    else {
      CorrespondingTreePtr nil = new CorrespondingTree ;
      nil->mToken = new Token ;
      nil->mToken->mTokenType = NIL ;
      
      return nil ;
    } // else
    
  } // CommandEqual()
  
  bool SameTree( CorrespondingTreePtr a, CorrespondingTreePtr b,
                 bool needSameAddress, int level, bool & error ) {
    if ( SameNode( a, b, needSameAddress, level, error ) ) {
      if ( a->mRightNode == NULL ) {
        
        return true ;
      } // if
      else {
        
        return ( SameTree( a->mLeftNode, b->mLeftNode, needSameAddress, level, error ) &&
                 SameTree( a->mRightNode, b->mRightNode, needSameAddress, level, error ) ) ;
      } // else
        
    } // if
    else {
     
      return false ;
    } // else
    
  } // SameTree()
  
  bool SameNode( CorrespondingTreePtr a, CorrespondingTreePtr b,
                 bool needSameAddress, int level, bool & error ) {
    if ( a->mToken != NULL ) {
      if ( b->mToken != NULL ) {
        if ( a->mToken->mTokenType == SYMBOL ) {
          if ( b->mToken->mTokenType == SYMBOL ) {
            if ( needSameAddress ) {
              
              return  GetBinding( a->mToken ) ==
                      GetBinding( b->mToken ) ;
            } // if ( needSameAddress )
            else {
              bool temp = false ;
              CorrespondingTreePtr tempA = GetBindingAndEval( a->mToken, temp, error, level ) ;
              if ( error ) {
                error = true ;
                
                return false ;
              } // if
              else {
                if ( tempA == GetBindingAndEval( b->mToken, temp, error, level ) ) {
                  if ( error ) {
                    
                    return false ;
                  } // if
                  else {
                    
                    return true ;
                  } // else
                  
                } // if
                else {
                  
                  return false ;
                } // else
                
              } // else
              
            } // else
            
          } // if ( b->mToken->mTokenType == SYMBOL )
          else {
            
            return false ;
          } // else
          
        } // if ( a->mToken->mTokenType == SYMBOL )
        else {
          if ( a->mToken->mTokenType == b->mToken->mTokenType ) {
            if ( a->mToken->mToken != NULL ) {
              if ( b->mToken->mToken != NULL ) {
                
                return ( strcmp( a->mToken->mToken,
                                 b->mToken->mToken ) == 0 ) ;
              } // if
              else {
                
                return false ;
              } // else
              
            } // if ( a->mToken->mToken != NULL )
            else {
              
              return ( a->mToken->mToken == b->mToken->mToken ) ;
            } // else
            
          } // if
          else {
            
            return false ;
          } // else
          
        } // else
        
      } // if
      else {
        
        return false ;
      } // else
      
    } // if ( a->mToken != NULL )
    else {
      if ( b->mToken != NULL ) {
        
        return false ;
      } // if
      else {
         
        return  true ;
      } // else
      
    } // else
    
  } // SameNode()
  
  CorrespondingTreePtr CommandBegin( CorrespondingTreePtr head, int level, bool & error ) {
    CorrespondingTreePtr walk = head ;
    CorrespondingTreePtr value = NULL ;
    
    while ( walk->mRightNode->mRightNode != NULL && ! error ) {
      error = Eval( walk->mLeftNode, value, level + 1 ) ;
      
    } // while
    
    if ( ! error ) {
      error = Eval( walk->mLeftNode, value, level + 1 ) ;
      if ( ! error ) {
        
        return value ;
      } // if
      else {
        value = NULL ;
        
        return value ;
      } // else
      
    } // if ( ! error )
    else {
      value = NULL ;
      
      return value ;
    } // else
    
  } // CommandBegin()
  
  CorrespondingTreePtr CommandIf( CorrespondingTreePtr head, int level, bool & error ) {
    CorrespondingTreePtr value = NULL ;
    error = false ;
    if ( Eval( head->mRightNode->mLeftNode, value, level + 1 ) ) {
      if ( value->mToken != NULL && value->mToken->mTokenType == NIL ) {
        if ( head->mRightNode->mRightNode->mRightNode->mLeftNode != NULL ) {
          if ( Eval( head->mRightNode->mRightNode->mRightNode->mLeftNode,
                     value, level + 1 ) ) {
            
            return value ;
          } // if
          else {
            error = true ;
            
            return NULL ;
          } // else
          
        } // if ( head->mRightNode->mRightNode->mLeftNode != NULL )
        else {
          Error temp ;
          temp.mErrorType = NOVALUE ;
          temp.mBinding = head ;
          
          mErrorVct->push_back( temp ) ;
          
          return NULL ;
        } // else
        
      } // if ( value->mToken != NULL && value->mToken->mTokenType == NIL )
      else {
        if ( head->mRightNode->mRightNode->mLeftNode != NULL ) {
          if ( Eval( head->mRightNode->mRightNode->mLeftNode,
                     value, level + 1 ) ) {
            
            return value ;
          } // if
          else {
            error = true ;
            
            return NULL ;
          } // else
          
        } // if ( head->mRightNode->mRightNode->mLeftNode != NULL )
        else {
          Error temp ;
          temp.mErrorType = NOVALUE ;
          temp.mBinding = head ;
          
          mErrorVct->push_back( temp ) ;
          
          return NULL ;
        } // else
        
      } // else
      
    } // if ( Eval( head->mLeftNode, value, level ) )
    else {
      error = true ;
      
      return NULL ;
    } // else
    
  } // CommandIf()
  
  CorrespondingTreePtr CommandCond( CorrespondingTreePtr head, int level, bool & error ) {
    CorrespondingTreePtr value = NULL ;
    CorrespondingTreePtr walk = head->mRightNode ;
    error = false ;
    bool found = false ;
    
    while ( walk->mRightNode != NULL && ! found && ! error ) {
      if ( walk->mRightNode->mRightNode == NULL &&
           walk->mLeftNode->mToken != NULL &&
           ( strcmp( walk->mLeftNode->mToken->mToken, "else" ) == 0 ) ) {
        
        value = CommandBegin( walk->mLeftNode->mRightNode, level, error ) ;
        found = true ;
      } // if
      else if ( Eval( walk->mLeftNode->mLeftNode, value, level + 1 ) ) {
        if ( value->mToken != NULL &&  value->mToken->mTokenType == NIL ) {
          
          walk = walk->mRightNode ;
        } // if
        else {
          
          value = CommandBegin( walk->mLeftNode->mRightNode, level, error ) ;
          found = true ;
        } // else
        
      } // else if ( Eval( walk->mLeftNode, value, level + 1 ) )
      else {
        
        error = true ;
      } // else
      
    } // while
    
    if ( error ) {
      
      return NULL ;
    } // if
    else {
      if ( found ) {
        
        return value ;
      } // if
      else {
        error = true ;
        Error temp ;
        temp.mErrorType = NOVALUE ;
        temp.mBinding = head ;
        
        mErrorVct->push_back( temp ) ;
        
        return NULL ;
      } // else
      
    } // else
    
  } // CommandCond()
  
  // proceed
  void CleanEnvironment() {
    mSymbolVct->clear() ;
    
    return ;
  } // CleanEnvironment()
  
  bool PlantCorrespondingTree( SExpressionPtr sExp, CorrespondingTreePtr & head ) {
    bool hasError = false ;
    head = NULL ;
    
    head = PlantTree( sExp->mTokenString, hasError ) ;
    
    if ( hasError ) {
      head = NULL ;
      
      return false ;
    } // if
    else { // if ( no Error )
      
      return true ;
    } // else
    
  } // PlantCorrespondingTree()
  
  CorrespondingTreePtr PlantTree( TokenPtr head, bool & error ) {
    
    if ( head->mTokenType == LEFTPAREN ) {
      if ( head->mNext->mTokenType == RIGHTPAREN ) {
        CorrespondingTreePtr temp = new CorrespondingTree ;
        TokenPtr nil = new Token ;
        nil->mTokenType = NIL ;
        temp->mToken = nil ;
        
        return temp ;
      } // if
      else {
        TokenPtr tail = head ;
        
        return GetSExpTree( head, tail, error, false ) ;
      } // else
      
    } // if
    else if ( head->mTokenType == QUOTE ) {
      TokenPtr tail = head ;
      
      return GetSExpTree( head, tail, error, false ) ;
    } // else if
    else if ( Scanner::IsAtom( head->mTokenType ) ) {
      CorrespondingTreePtr temp = new CorrespondingTree ;
      temp->mToken = head ;
      
      return temp ;
    } // else if
    else {
      error = true ;
      
      return NULL ;
    } // else
    
  } // PlantTree()
  
  CorrespondingTreePtr GetSExpTree( TokenPtr head, TokenPtr & tail, bool & error, bool fromQuote ) {
    
    if ( head == NULL ) {
      return NULL ;
    } // if
    else if ( head->mTokenType == LEFTPAREN ) {
      
      if ( head->mNext->mTokenType == RIGHTPAREN ) {
        tail = head->mNext->mNext ;
        return Cons( GetSExpTree( head->mNext, tail, error, fromQuote ),
                     GetSExpTree( tail, tail, error, fromQuote ) ) ;
      } // if
      else { // if ( head->mNext->mTokenType != RIGHTPAREN )
        CorrespondingTreePtr tempA = GetSExpTree( head->mNext, tail, error, fromQuote ) ;
        
        CorrespondingTreePtr tempB = GetSExpTree( tail, tail, error, fromQuote ) ;
              
        if ( tempB != NULL ) {
          
          return Cons( tempA, tempB ) ;
        } // if
        else {
          
          return tempA ;
        } // else
        
      } // else
      
    } // else if
    else if ( Scanner::IsAtom( head->mTokenType ) ) {
      CorrespondingTreePtr temp = new CorrespondingTree ;
      temp->mToken = head ;
      
      return Cons( temp, GetSExpTree( head->mNext, tail, error, fromQuote ) ) ;
    } // else if
    else if ( head->mTokenType == QUOTE ) {
      CorrespondingTreePtr temp = new CorrespondingTree ;
      temp->mToken = head ;
      CorrespondingTreePtr nilTree = new CorrespondingTree ;
      TokenPtr nil = new Token ;
      nil->mTokenType = NIL ;
      nilTree->mToken = nil ;
      
      if ( head->mNext->mTokenType == LEFTPAREN ) {
        
        CorrespondingTreePtr tempA = Cons( temp,
                                           Cons( GetSExpTree( head->mNext->mNext, tail, error, false ),
                                                 nilTree ) ) ;
        if ( ! fromQuote ) {
          CorrespondingTreePtr tempB = GetSExpTree( tail, tail, error, false ) ;
          
          if ( tempB != NULL ) {
            
            return Cons( tempA, tempB ) ;
          } // if
          else {
            
            return tempA ;
          } // else
          
          return Cons( tempA, tempB ) ;
        } // if
        else {
          
          return tempA ;
        } // else
        
      } // if
      else if ( head->mNext->mTokenType == QUOTE ) {
        CorrespondingTreePtr tempA = Cons( temp,
                                           Cons( GetSExpTree( head->mNext, tail, error, true ),
                                                 nilTree ) ) ;
        if ( ! fromQuote ) {
          CorrespondingTreePtr tempB = GetSExpTree( tail, tail, error, false ) ;
          
          if ( tempB != NULL ) {
            
            return Cons( tempA, tempB ) ;
          } // if
          else {
            
            return tempA ;
          } // else
          
        } // if
        else {
          
          return tempA ;
        } // else
        
      } // else if
      else { // should be atom
        CorrespondingTreePtr atomTree = new CorrespondingTree ;
        atomTree->mToken = head->mNext ;
        tail = head->mNext->mNext ;
        
        if ( ! fromQuote ) {
          CorrespondingTreePtr tempB = GetSExpTree( tail, tail, error, false ) ;
          if ( tempB != NULL ) {
            
            return Cons( Cons( temp, Cons( atomTree, nilTree ) ), tempB ) ;
          } // if
          else {
            
            return Cons( temp, Cons( atomTree, nilTree ) ) ;
          } // else
          
        } // if
        else {
          
          return Cons( temp, Cons( atomTree, nilTree ) ) ;
        } // else
        
      } // else
      
    } // else if
    else if ( head->mTokenType == DOT ) {
      
      if ( head->mNext->mTokenType == LEFTPAREN ) {
        CorrespondingTreePtr temp = GetSExpTree( head->mNext->mNext, tail, error, fromQuote ) ;
        tail = tail->mNext ;
        
        return temp ;
      } // if
      else if ( Scanner::IsAtom( head->mNext->mTokenType ) ) {
        CorrespondingTreePtr temp = new CorrespondingTree ;
        temp->mToken = head->mNext ;
        tail = head->mNext->mNext->mNext ;
        
        return temp ;
      } // else if
      else {
        cout << "here are some error I don't know." << endl ;
        error = true ;
        
        return NULL ;
      } // else
      
    } // else if
    else if ( head->mTokenType == RIGHTPAREN ) {
      
      CorrespondingTreePtr temp = new CorrespondingTree ;
      
      TokenPtr nil = new Token ;
      nil->mTokenType = NIL ;
      temp->mToken = nil ;
      
      tail = head->mNext ;
      
      return temp ;
    } // else if
    else {
      cout << "here are some error I don't know." << endl ;
      error = true ;
      
      return NULL ;
    } // else
    
  } // GetSExpTree()
    
  CorrespondingTreePtr Cons( CorrespondingTreePtr a, CorrespondingTreePtr b ) {
    CorrespondingTreePtr c = new CorrespondingTree ;
    
    c->mLeftNode = a ;
    c->mRightNode = b ;
    
    return c ;
  } // Cons()
  
  bool IsExit( TokenPtr tokenString ) {
    if ( tokenString != NULL &&
         tokenString->mTokenType == LEFTPAREN &&
         tokenString->mNext != NULL &&
         strcmp( tokenString->mNext->mToken, "exit" ) == 0 &&
         tokenString->mNext->mNext != NULL &&
         tokenString->mNext->mNext->mTokenType == RIGHTPAREN ) {
      
      cout << endl ;
      return true ;
    } // if
    else {
      
      return false ;
    } // else
        
  } // IsExit()
    
  void PrintError() {
    int len = mErrorVct->size() ;
    
    for ( int i = 0 ; i < len ; i++ ) {
      if ( mErrorVct->at( i ).mErrorType == UNBOUND ) {
        
        cout << "ERROR (unbound symbol) : " << mErrorVct->at( i ).mToken << endl ;
      } // if ( mErrorVct->at( i ).mErrorType == UNBOUND )
      else if ( mErrorVct->at( i ).mErrorType == NONLIST ) {
        
        cout << "ERROR (non-list) : " ;
        PrintCorrespondingTree( mErrorVct->at( i ).mBinding, 0, true ) ;
      } // else if ( mErrorVct->at( i ).mErrorType == NONLIST )
      else if ( mErrorVct->at( i ).mErrorType == APPLYNONFUNC ) {
        
        cout << "ERROR (attempt to apply non-function) : "
             << mErrorVct->at( i ).mToken << endl ;
      } // else if ( mErrorVct->at( i ).mErrorType == APPLYNONFUNC )
      else if ( mErrorVct->at( i ).mErrorType == LEVEL ) {
        
        cout << "ERROR (level of " ;
        if ( mErrorVct->at( i ).mToken == "clean-environment" == 0 ) {
          cout << "CLEAN-ENVIRONMENNT)" << endl ;
        } // if
        else if ( mErrorVct->at( i ).mToken == "define" ) {
          cout << "DEFINE)" << endl ;
        } // else if
        else if ( mErrorVct->at( i ).mToken == "exit" ) {
          cout << "EXIT)" << endl ;
        } // else if
        
      } // else if ( mErrorVct->at( i ).mErrorType == LEVEL )
      else if ( mErrorVct->at( i ).mErrorType == FORMAT ) {
        
        cout << "ERROR (" ;
        if ( strcmp( mErrorVct->at( i ).mBinding->mLeftNode->mToken->mToken,
                     "define" ) == 0 ) {
          cout << "DEFINE format) : " ;
        } // if
        else if ( strcmp( mErrorVct->at( i ).mBinding->mLeftNode->mToken->mToken,
                          "cond" ) == 0 ) {
          cout << "COND format) : " ;
        } // else if
        
        PrintCorrespondingTree( mErrorVct->at( i ).mBinding, 0, true ) ;
        
      } // else if ( mErrorVct->at( i ).mErrorType == FORMAT )
      else if ( mErrorVct->at( i ).mErrorType == ARGUMENTS ) {
        
        cout << "ERROR (incorrect number of arguments) : "
             << mErrorVct->at( i ).mToken << endl ;
      } // else if ( mErrorVct->at( i ).mErrorType == ARGUMENTS )
      else if ( mErrorVct->at( i ).mErrorType == NONFUNCTION ) {
        
        cout << "ERROR (attempt to apply non-function) : " ;
        PrintCorrespondingTree( mErrorVct->at( i ).mBinding, 0, true ) ;
      } // else if ( mErrorVct->at( i ).mErrorType == NONFUNCTION )
      else if ( mErrorVct->at( i ).mErrorType == ARGTYPE ) {
        
        cout << "ERROR (" << mErrorVct->at( i ).mToken
             << " with incorrect argument type) : "
             << mErrorVct->at( i ).mBinding->mToken->mToken << endl ;
      } // else if ( mErrorVct->at( i ).mErrorType == ARGTYPE )
      else if ( mErrorVct->at( i ).mErrorType == NOVALUE ) {
        
        cout << "ERROR (no return value) : " ;
        PrintCorrespondingTree( mErrorVct->at( i ).mBinding, 0, true ) ;
      } // else if ( mErrorVct->at( i ).mErrorType == NOVALUE )
      else {
        
        cout << "should not be here !(printError)" << endl ;
      } // else
      
    } // for
    
    mErrorVct->clear() ;
    
    return ;
  } // PrintError()
    
} ; // class parser

// ---       general function  ---

int main() {
    
  Scanner scanner ;
  Parser parser ;
  
  int inputID ;
  
  bool notEnd = true ;
  bool hasEof = false ;
  bool hasError = false ;
  
  SExpressionPtr sExpPtr = NULL ;
  
  CorrespondingTreePtr correspondingTreePtr = NULL ;
  
  CorrespondingTreePtr value = NULL ;
  
  cout << fixed << setprecision( 3 ) ;
  
  cout << "Welcome to OurScheme!" << endl ;
  
  cin >> inputID ;
  
  while ( notEnd && ! hasEof && inputID == 1 ) {    // not exit && not EOF
    cout << endl << "> " ;
    
    if ( scanner.ReadSExp( sExpPtr ) ) {
      
      notEnd = ! parser.IsExit( sExpPtr->mTokenString ) ;
      if ( notEnd ) {
        if ( parser.PlantCorrespondingTree( sExpPtr, correspondingTreePtr ) ) {
          
          if ( parser.Eval( correspondingTreePtr, value, 0 ) ) {
            if ( notEnd ) {
              // parser.CheckTree( correspondingTreePtr ) ;
              int space = 0 ;
              parser.PrintCorrespondingTree( value, space, true ) ; // pretty print
            } // if
            
          } // if
          else { // Eval() has Error
            parser.PrintError() ;
          } // else
          
        } // if
        else { // if PlantCorrespondingTree() has Error
          cout << "Has Error when planting tree." << endl ;
        } // else
        
      } // if
      
    } // if
    else { // scanner has Error () )
      scanner.PrintError( hasEof ) ;
    } // else
      
    value = NULL ;
    correspondingTreePtr = NULL ;
    sExpPtr = NULL ;
    
  } // while
  
  cout << "Thanks for using OurScheme!" ;
    
  return 0 ;
} // main()

