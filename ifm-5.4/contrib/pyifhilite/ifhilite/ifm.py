"""
IFM syntax highlighting.
"""

from pygments.lexer import RegexLexer
from pygments.token import (Text, Comment, Operator, Keyword, Name,
                            String, Number, Punctuation, Token)

structure = ["room"]

specials = ["title", "map", "require", "style", "endstyle"]

builtins = ["it", "them", "last", "true", "false", "undef"]

keywords = ["all", "after", "any", "before", "cmd", "d", "do", "down",
            "dir", "drop", "endstyle", "except", "exit", "finish", "follow",
            "from", "get", "give", "go", "goto", "hidden", "ignore", "in",
            "item", "join", "keep", "leave", "length", "link", "lose",
            "lost", "map", "need", "none", "nodrop", "nolink", "nopath",
            "note", "oneway", "out", "require", "room", "safe", "score",
            "start", "style", "tag", "task", "title", "to", "u", "up",
            "until", "with"]

directions = ["n", "north", "ne", "northeast", "e", "east",
              "se", "southeast", "s", "south", "sw", "southwest",
              "w", "west", "nw", "northwest"]

obsolete = ["given", "times"]

def wordlist(list):
    return "(" + "|".join(list) + r")\b"

class IFMLexer(RegexLexer):
    """
    IFM code lexer.
    """

    name = 'Interactive Fiction Mapper'
    aliases = ['ifm', 'IFM']
    filenames = ['*.ifm']
    mimetypes = ['text/x-ifm', 'application/x-ifm']

    tokens = {
        'root': [
            (r'\n', Text),
            (r'[^\S\n]+', Text),
            (r'#.*$', Comment),
            (r';', Punctuation),
            (r'\\\n', Text),
            (r'\\', Text),
            (r'=', Operator),
            (r'"[^"]*"', String),
            (r'^.+?=', Token.Token),

            ("^" + wordlist(structure), Name.Class),
            ("^" + wordlist(specials), Name.Entity),

            (wordlist(keywords), Token.Keyword.Reserved),
            (wordlist(builtins), Name.Builtin),
            (wordlist(directions), Name.Variable),
            (wordlist(obsolete), Token.Generic.Error),

            (r'(\d+\.?\d*|\d*\.\d+)([eE][+-]?[0-9]+)?', Number.Float),
            (r'\d+', Number.Integer),
            (r'[a-zA-Z_][a-zA-Z0-9_.]*', Name),
        ],
    }
