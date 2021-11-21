""" Classes for representing and formatting http Link entity-headers """

from itertools import chain

class LinkParam:
    """ Represents a Link parameter-value pair """
    param: str
    value: str

    def __init__(self, param: str, value: str) -> None:
        self.param = param
        self.value = value

    def __str__(self) -> str:
        return f'{self.param}={self.value}'

    def __repr__(self) -> str:
        return str(self)

class Link:
    """ Represents and individual link, including it's parameters """
    ref: str
    params: list[LinkParam]

    def __init__(self, ref: str, *params: LinkParam) -> None:
        self.ref = ref
        self.params = list(params)

    def __str__(self) -> str:
        return '; '.join(chain([f'<{self.ref}>'], map(str, self.params)))

    def __repr__(self) -> str:
        return str(self)

class LinkHeader:
    """ Represents a Link header, a list of links """
    links: list[Link]

    def __init__(self, *links: Link) -> None:
        self.links = list(links)

    def __str__(self) -> str:
        return ', '.join(map(str, self.links))

    def __repr__(self) -> str:
        return str(self)
