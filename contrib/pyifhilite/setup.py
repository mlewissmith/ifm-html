"""
Interactive Fiction syntax highlighting for Pygments.
"""

from setuptools import setup

entry_points = """
[pygments.lexers]
ifm = ifhilite.ifm:IFMLexer
transcript = ifhilite.transcript:TranscriptLexer
"""

setup(
    name         = 'pyifhilite',
    version      = '0.1',
    description  = __doc__.strip(),
    author       = "Glenn Hutchings",
    author_email = "zondo42@googlemail.com",
    packages     = ['ifhilite'],
    entry_points = entry_points
)
