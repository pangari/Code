#ifndef _GLSTRING_HPP
#define _GLSTRING_HPP

#include <string.h>
#include <stdlib.h>
#include <limits.h>

#define NULLSTRING          GLString()
#define MAX_LEN             UINT_MAX

#include <list>
#include <string>

class GLString;
typedef std::list<GLString> GLStringList;

/**
	GLString class handles character string manipulation.
*/
class GLString
{
protected:
	std::string m_str;

public :
	GLString( ) {};
	GLString(const std::string &str):m_str(str) {}
/**
	Constructor - allocate a fixed size with a padding character.


	@param		_size	string length
	@param		_c		padding character (default '\0')
*/

	explicit GLString (int size, const char _c = '\0'):m_str(size,_c) {}
/**
	Constructor - build a GLString from a simple character
	the size of the created string is 1.

	@param		_c	character

	@see

*/
	explicit GLString (const char _c):m_str(1,_c) {}
	explicit GLString (const char * _pc):m_str(_pc) {}
	GLString (const char * _pc, unsigned int _len):m_str(_pc,_len) {}

	// erases the entire string.
	void clear(){m_str.clear();}
	// true if string's size is 0.
	bool empty(){return m_str.empty();}

/**
	Constructor - build a GLString from a C character string.


	@param		_pc	character string (ending with 0)
	@param		_ofs	starting offset in character string
	@param		_len	length to copy
*/
	GLString (const char * _pc, unsigned int _ofs, unsigned int _len):m_str(_pc+_ofs,_len) {}

    GLString& operator= ( const GLString& _s ) {
		m_str=_s.m_str;
		return *this;
	}
	GLString& operator= ( const char * _pc ) {
		m_str=_pc;
		return *this;
	}

	GLString  operator+ ( const GLString& _s ) const { return m_str+_s.m_str; }
	GLString  operator+ ( const char * _pc ) const { return m_str+_pc; }
	GLString  operator+ ( const char c ) const { return m_str+c; }
	friend  GLString  operator+ ( const char *, const GLString& );
	friend  GLString  operator+ ( const char , const GLString& );

	GLString& operator+= ( const GLString& _s ) {
		m_str+=_s.m_str;
		return *this;
	}
	GLString& operator+= ( const char * _pc ) {
		m_str+=_pc;
		return *this;
	}

	bool operator==( const char * _pc   ) const { return m_str==_pc; }
	bool operator==( const GLString& _s ) const { return m_str==_s.m_str; }
	bool operator!=( const char * _pc   ) const { return m_str!=_pc; }
	bool operator!=( const GLString& _s ) const { return m_str!=_s.m_str; }
	bool operator< ( const char * _pc   ) const { return m_str< _pc; }
	bool operator< ( const GLString& _s ) const { return m_str< _s.m_str; }
	bool operator> ( const char * _pc   ) const { return m_str> _pc; }
	bool operator> ( const GLString& _s ) const { return m_str> _s.m_str; }
	bool operator<=( const char * _pc   ) const { return m_str<=_pc; }
	bool operator<=( const GLString& _s ) const { return m_str<=_s.m_str; }
	bool operator>=( const char * _pc   ) const { return m_str>=_pc; }
	bool operator>=( const GLString& _s ) const { return m_str>=_s.m_str; }


	friend bool operator==( const char * _pc, const GLString & );
	friend bool operator!=( const char * _pc, const GLString & );
	friend bool operator< ( const char * _pc, const GLString & );
	friend bool operator> ( const char * _pc, const GLString & );
	friend bool operator<=( const char * _pc, const GLString & );
	friend bool operator>=( const char * _pc, const GLString & );


/**
	Make an alpha comparison between a GLString and a charactre string. Called by all the comparison operators
	==, <, >, <= and >= returning bool.

	@param		_pc	String to compare

	@return		0 if the two string are identical, -1 if the GLString is less than the char *;
				+1 if the GLString is greater than the char *

	@see

*/
	int  Compare( const char *_pc ) const {
		int result=m_str.compare(_pc);
		return result<0?-1:(result>0?1:0);
	}
	int  Compare( const GLString& _s ) const {
		int result=m_str.compare(_s.m_str);
		return result<0?-1:(result>0?1:0);
	}

	unsigned int  Length() const { return (int)m_str.size(); }
	char    operator[] ( int _n ) const { return m_str[_n]; }
	const char*   c_str() const { return m_str.c_str(); }
	const char*   c_str(unsigned int _pos) const { return m_str.c_str()+_pos; }
	operator const char* () const { return m_str.empty()?0:m_str.c_str(); }
	std::string &std_str() { return m_str; }
	const std::string &std_str() const { return m_str; }
/**
	Build a GLString based on the first characters of another GLString
	(Ex : GLString s = "ABCDEF"; GLString l = s.Left( 4 ) will give "ABCD"

	@param		lg	length to extract

	@return		the new GLString

	@see

*/
	GLString  Left( unsigned int _lg ) const { return std::string(m_str,0,_lg); }
/**
	Build a GLString based on the last characters of another GLString
	(Ex : GLString s = "ABCDEF"; GLString l = s.Right( 4 ) will give "CDEF"

	@param		lg	length to extract

	@return		the new GLString

	@see

*/
	GLString  Right( unsigned int _lg ) const { return std::string(m_str,m_str.size()-_lg,std::string::npos); }
/**
	Build a GLString from a substring
	(Ex : GLString s = "ABCDEF"; GLString l = s.SubString( 2,2 ) will give "CD"

	@param		pos	starting position
	@param		len	length

	@return		the new string

	@see

*/
	GLString  SubString( unsigned int _pos, unsigned int _len = MAX_LEN ) const {
		return m_str.substr(_pos,_len==MAX_LEN?std::string::npos:_len);
	}


/**
	Make an comparison between a GLString and a character string. the strings can used '*' and '?' jocker .

	Exemple : "ABC*"  == "ABCDEFG"
			  "AB?D" ==  "ABCD"
			  "A*xxx"   ==  "AB*yyyy"
			  "ABCD?FG" != "A?CDEFx"


	@param		_pc	GLString to compare

	@return		1 if the two string are identical.
				0 if the two string are different.
	@see

*/

	int  ExtendCompare( const char *_pc ) const { return ExtendCompare(m_str,_pc); }
/**
	Make an comparison between a GLString and a character string. the strings can used '*' and '?' jocker .

	Exemple : "ABC*"  == "ABCDEFG"
			  "AB?D" ==  "ABCD"
			  "A*xxx"   ==  "AB*yyyy"
			  "ABCD?FG" != "A?CDEFx"


	@param		s	GLString to compare

	@return		1 if the two string are identical.
				0 if the two string are different.

	@see

*/
	int  ExtendCompare( const GLString& _s ) const { return ExtendCompare(m_str,_s.m_str); }
/**
	Convert a string to uppercase.


	@return		the modified GLString

	@see

*/
	GLString& ToUpper() { m_str=ToUpper(m_str); return *this; }


/**
	Concert a string to lowercase.


	@return		the modified GLString

	@see

*/
	GLString& ToLower() { m_str=ToLower(m_str); return *this; }


/**
	Returns the index of the first occurence of a substring in another
	(Index only returns the first occurence of the substring. To get the
	other occurence, use GLString::Parse).


	@param		_pc				C string to be located
	@param		_sensitive		true : case sensitive, false - no distinction between case

	@return		Position of the string to be located. Returns 0 if the GLString starts with the substring.
				Returns 0xFFFF or -1 if the substring doesn't exist in the GLString

	@see		GLString::Parse

*/
	unsigned int  Index( const char * _pc, bool _sensitive = true  ) const {
		std::string::size_type result=Index(m_str, _pc,  _sensitive);
		return result==std::string::npos?(unsigned int)(-1):result;
	}
/**
	returns the index of the first occurence of a GLString in another
	(Index only returns the first occurence of the substring. To get the
	other occurence, use GLString::Parse).
.

	@param		s			character string to be located
	@param		sensitive	true if case sensitive, false otherwise

	@return		Position of the string to be located. Returns 0 if the GLString starts with the substring.
				Returns NOT_FOUND or -1 if the substring doesn't exist in the GLString

	@see
*/
	unsigned int  Index( const GLString& _s, bool _sensitive = true ) const {
		std::string::size_type result=Index(m_str,_s.m_str,_sensitive);
		return result==std::string::npos?(unsigned int)(-1):result;
	}
/**
	Returns the index of the first occurence of a character in a GLString
	(Index only returns the first occurence of the character. To get the
	other occurence, use GLString::Parse).


	@param		_c			character to be located
	@param		_sensitive	true : case sensitive, false - no distinction between case

	@return		Position of the character to be located. Returns 0 if the GLString starts with the character.
				Returns 0xFFFF or -1 if the character doesn't exist in the GLString

	@see		GLString::Parse

*/
	unsigned int Index( const char _c, bool _sensitive = true ) const {
		std::string::size_type result=Index(m_str,std::string(1,_c),_sensitive);
		return result==std::string::npos?(unsigned int)(-1):result;
	}

	unsigned int IndexFromEnd( const char * _pc, bool _sensitive= true ) const {
		std::string::size_type result=IndexFromEnd(m_str,_pc,_sensitive);
		return result==std::string::npos?(unsigned int)(-1):result;
	}

	GLString ParseFromEnd( const char* _sep, bool _sensitive= true, bool _stripwsp = true) {
		return ParseFromEnd(m_str,_sep,_sensitive,_stripwsp);
	}

/**
	Parse a GLString in }the REXX way
	(ex : GLString s =  " ABCD = 1234 "; GLString left = s.Parse( "=" )
	 we have left = "ABCD" and s = "1234").


	@param		_sep			separator (C string ending with 0)
	@param		_sensitive		true : case sensitive, false - no distinction between case
	@param		_stripwsp		true : strip blanks (default), false - don't strip blanks

	@return		the string on the left of the separator. The calling GLString if the separator doesn't exist.
				After PARSE, the calling GLString is the part on the right of the separator of nothing if the
				separator doesn't exist

	@see

*/
	GLString  Parse( const char * _psep, bool _sensitive = true, bool _stripwsp = true ) {
		return Parse(m_str,_psep,_sensitive,_stripwsp);
	}
/**
	Cut out a GLString in the REXX way
	(Ex : GLString s =  " ABCD = 1234 "; GLString left = s.Parse( "=" )
	 will give left = "ABCD" and s = "1234").

	@param		s			GLString separator
	@param		sensitive	true case sensitive (default), false otherwise
	@param		stripwsp	true strip blanks (default), false otherwise

	@return		the string on the left of the separator. The calling GLString if the separator doesn't exist.
				After PARSE, the calling GLString is the part on the right of the separator of nothing if the
				separator doesn't exist

	@see

*/
	GLString  Parse( const GLString& _sep, bool _sensitive = true, bool _stripwsp = true ) {
		return Parse(m_str,_sep.m_str,_sensitive,_stripwsp);
	}
/**
	PARSE a GLString in the REXX way
	(ex : GLString s =  " ABCD = 1234 "; GLString left = s.Parse( '=' )
	 we have left = "ABCD" and s = "1234").


	@param		_c				separator character
	@param		_sensitive		true : case sensitive, false - no distinction between case
	@param		_stripwsp		true : strip blanks (default), false - don't strip blanks

	@return		the string on the left of the separator. The calling GLString if the separator doesn't exist.
				After PARSE, the calling GLString is the part on the right of the separator of nothing if the
				separator doesn't exist

	@see

*/
	GLString  Parse( const char _c, bool _sensitive = true, bool _stripwsp = true ) {
		return Parse(m_str,std::string(1,_c),_sensitive,_stripwsp);
	}

/**
	Remove the leading and/or trailing character on a GLString
	(Ex : if GLString s = "    Hello   ", s.Strip() returns "Hello".


	@param		_stripchar	character to be removed
				_side		' '	remove the leading and trailing characters
							'L'	remove the leading characters
							'T'	remove the trailing characters
	@return		the GLString destination

	@see

*/

	GLString& Strip( const char _stripchar = ' ', char _side = ' ' ) {
		m_str=Strip(m_str,_stripchar,_side);
		return *this;
	}

/**
	replace all the occurence of a substring by another in a GLString
	( Ex :	GLString s = "AB CD EF AB"
			s.Replace( "AB", "XX" )  -> s : "XX CD EF XX" ).

	@param		*_szold			substring to replace
	@param		*_sznew			replacing substring
	@param		_sensitive		true : case sensitive, false - no distinction between case

	@return		destination GLString

	@see

*/

	GLString& Replace( const char * _pszold, const char * _psznew, bool _sensitive = true ) {
		m_str=Replace(m_str,_pszold,_psznew,_sensitive);
		return *this;
	}


/**
	Overlay a portion of a GLString with a new string.
	(Ex :	GLString s = "ABCDEF";
			s.Overlay( "XX",2 ) ->	"ABXXEF").

	@param		*_pc		replacing string
	@param		_position	position of beginning of replacement
	@param		_len		length of replacement (default : replacing string length)

	@return		destination string

	@see

*/

	GLString& Overlay( const char *_pc, unsigned int position, unsigned int _len = MAX_LEN ) {
		m_str=Overlay(m_str,_pc,position,_len==MAX_LEN?std::string::npos:_len);
		return *this;
	}
/**
	Overlay a portion of a GLString with a new string
	(Ex :	GLString s = "ABCDEF";
			s.Overlay( "XX",2 ) ->	"ABXXEF").


	@param		s			replacing string
	@param		position	position of beginning of replacement
	@param		len			length of replacement (default : replacing string length)

	@return		destination string

	@see

*/
	GLString& Overlay( const GLString& _s, unsigned int _position, unsigned int _len = MAX_LEN ) {
		m_str=Overlay(m_str,_s.m_str,_position,_len==MAX_LEN?std::string::npos:_len);
		return *this;
	}


/**
	Returns the n th word of a GLString (it doesn't work like Word : adjacent separators are not
	considered as one)
	(Ex	:  = "AB\t\tCD\tEF"; s.Extract( 2 ) returns ""
							 s.Extract( 3 ) returns "CD".


	@param		_n		word position
	@param		_sep	word separator (default \t)

	@return		the word

	@see

*/

	GLString  Extract( unsigned int _n, const char _sep = '\t' ) const {
		return Extract(m_str,_n,std::string(1,_sep));
	}

/**
	Returns the index of the word at a position n
	(Ex : GLString s = "AB  CD  EF"; s.WordIndex( 1 ) returns 4.

	@param		_n		word position
	@param		_sep	word separator character (default : blank)

	@return		position of the first letter of the word (first character = 0); -1 if the string is empty

	@see

*/

	unsigned int  WordIndex( unsigned int _n, const char _sep = ' ' ) const {
		std::string::size_type result=WordIndex(m_str,_n,std::string(1,_sep));
		return result==std::string::npos?(unsigned int)(-1):result;
	}
/**
	Returns the index of the word at the position n
	( Ex ! GLString s = "AB XX CD XX EF"; s.WordIndex( 1, " X" ) returns 6.


	@param		_n		word position
	@param		_sep	character set used as word separator (default blank)

	@return		position of the first letter of the word (first character = 0); -1 if the string is empty

	@see

*/
	unsigned int  WordIndex( unsigned int _n, const char * _psep ) const {
		std::string::size_type result= WordIndex(m_str,_n,_psep);
		return result==std::string::npos?(unsigned int)(-1):result;
	}

/**
	Returns the word at postion n
	(Ex : GLString s = "AB  CD  EF"; s.Word( 1 ) returns "CD".


	@param		_n		word position
	@param		_sep	word separator character (default : blank)

	@return		the word

	@see

*/
	GLString  Word( unsigned int _n, const char _sep = ' ' ) const {
		return Word(m_str,_n,std::string(1,_sep));
	}

/**
	Returns the word at position n
	(Ex : GLString s = "AB XX CD XX EF"; s.Word( 1, " X" )  returns "CD".


	@param		_n		word position
	@param		_psep	character set used as word separator (default blank)

	@return		the word

	@see

*/
	GLString  Word( unsigned int _n, const char * _psep ) const {
		return Word(m_str,_n,_psep);
	}
/**
    splits a string into a string list, according to a given separator
    ex : if the string to be split contains "this.is.a.string.to.split",
    _mystring.Split(".") will return a list containing the following 6 elements :
    this
    is
    a
    string
    to
    split

    IMPORTANT : the returned string list is allocated by the Split() function, and MUST be freed by the caller.
    The list can be manipulated by the usual STL list functions :

    @param		_sep			separator (C string ending with 0)
	@param		_sensitive		true : case sensitive, false - no distinction between case
	@param		_stripwsp		true : strip blanks (default), false - don't strip blanks

	@return		the allocated string list containing the different parts of the stripped string.
                After calling Parse, the original string is unchanged, as opposed to the Parse() function

	@see    GLString::Parse

    @example
                GLString From = "this.is.a.string.to.parse";
                GLStringList::iterator i;
                GLStringList *_List = From.Split('.');

                for (i=_List->begin(); i != _List->end(); ++i) cout << (const char *)*i << " ";
                 delete _List;
*/
	GLStringList *Split(const char * _psep, bool _sensitive = true, bool _stripwsp = true);

	GLStringList *Split(const GLString& _sep, bool _sensitive = true, bool _stripwsp = true);
/**
    splits a string into a string list, according to a given separator
    ex : if the string to be split contains "this.is.a.string.to.split",
    _mystring.Split(".") will return a list containing the following 6 elements :
    this
    is
    a
    string
    to
    split

    IMPORTANT : the returned string list is allocated by the Split() function, and MUST be freed by the caller.
    The list can be manipulated by the usual STL list functions :

	@param		_c				separator character
	@param		_sensitive		true : case sensitive, false - no distinction between case
	@param		_stripwsp		true : strip blanks (default), false - don't strip blanks

	@return		the allocated string list containing the different parts of the stripped string.
                After calling Parse, the original string is unchanged, as opposed to the Parse() function

	@see    GLString::Parse

    @example
                GLString From = "this.is.a.string.to.parse";
                GLStringList::iterator i;
                GLStringList *_List = From.Split('.');

                for (i=_List->begin(); i != _List->end(); ++i) cout << (const char *)*i << " ";
                 delete _List;
*/
	GLStringList *Split(const char _c, bool _sensitive = true, bool _stripwsp = true);

/**
	returns the value of a GLString representing an integer
	(Ex : GLString s = "14"; s.AsInt() returns 14. If s="E", s.AsInt() returns 0).

	@return		0 if the string can't be converted

	@see

*/

	int AsInt( ) const { return AsInt(m_str); }
/**
	returns the value of a GLString representing a long
	(Ex : GLString s = "-14"; s.AsLong() returns -14L. If s="E", s.AsLong() returns 0L).

	@return		long 0L if the string can't be converted

	@see

*/

	long AsLong( ) const { return AsLong(m_str); }
/**
	returns the value of a GLString representing a float
	(Ex : GLString s = "14.3"; s.AsDouble() returns -14.3. If s="X", s.AsDouble() returns 0.0).


	@return		double 0.0 if the string can't be converted

	@see

*/
	double AsDouble( ) const { return AsDouble(m_str); }
	/*
	   std::string Wrap
	*/
	static bool ExtendCompare(const std::string &, const std::string & );

	static std::string ToUpper(const std::string &);
	static std::string ToLower(const std::string &);

	static std::string::size_type Index(const std::string &src, const std::string & substr, bool sensitive );
	static std::string::size_type IndexFromEnd(const std::string &src, const std::string &substr, bool sensitive);
	static std::string ParseFromEnd(std::string &rightResult, const std::string &sep, bool sensitive= true, bool stripwsp = true);
	static std::string Parse(std::string &rightResult, const std::string & sep, bool sensitive = true, bool stripwsp = true );
	static std::string Strip(const std::string &src, const char stripchar = ' ', char side = ' ' );
	static int Replace(std::string &src, const std::string &oldStr, const std::string &newStr, bool sensitive = true );
	static std::string Overlay(const std::string &src, const std::string &dst, std::string::size_type _position, std::string::size_type _len = std::string::npos );
	static std::string Extract(const std::string &,std::string::size_type  _n, const std::string &sep = std::string(1,(char)1));
	static std::string::size_type  WordIndex(const std::string &, std::string::size_type _n, const std::string & sep );
	static std::string Word(const std::string &src, std::string::size_type n, const std::string & sep );

	static int AsInt(const std::string &str) { return str.empty()?0:atoi(str.c_str()); }
	static long AsLong(const std::string &str) { return str.empty()?0L:atol(str.c_str());}
	static double AsDouble(const std::string &str) {return str.empty()?0.0:atof(str.c_str());}

	static bool Test();
private:
	static void _TestError(const char *);
};



/**
	printf like.


	@param		_fmt		format (printf way)
	@param		...		variable list of arguments

	@return		result string

	@see

*/

std::string Format( const char *_fmt, ... );


inline GLString operator+ ( const char c, const GLString& s )
{
    return GLString( c ) + s;
}


inline GLString operator+ ( const char *pc, const GLString& s )
{
    return GLString( pc ) + s ;
}

inline bool operator==( const char *pc, const GLString &s )
{
	return pc==s.m_str;
}
inline bool operator!=( const char *pc, const GLString &s )
{
	return pc!=s.m_str;
}
inline bool operator< ( const char *pc, const GLString &s )
{
	return pc<s.m_str;
}
inline bool operator> ( const char *pc, const GLString &s )
{
	return pc>s.m_str;
}
inline bool operator<=( const char *pc, const GLString &s )
{
	return pc<=s.m_str;
}
inline bool operator>=( const char *pc, const GLString &s )
{
	return pc>=s.m_str;
}

#endif
