from typing import TypedDict

HTML_ATTRIBUTE_VALUE_PAIRS = dict[str, str]

class HTML_TAG_NAME_AND_ATTRIBUTES(TypedDict):
    tag_name: str
    """ The name of the tag. Ex: "div" or "table\""""
    attributes: HTML_ATTRIBUTE_VALUE_PAIRS
    """ A dictionary of attributes and the values of those attributes that the tag has.
    Ex: If the tag_name you gave was "div", and you wanted to find all the "div"s with "class" equal to "all_batting",
    the attributes would be {"class": "all_batting"}.
    """


HTML_TAG_NAVIGATION_PATH = list[HTML_TAG_NAME_AND_ATTRIBUTES]
""" A list of tag descriptors that leads to (and ends in) the tag we are looking for. """


class HTML_TAG_VALUE_LOCATION(TypedDict):
    """ The location of an attribute of an html tag. """
    tag_navigation_path: HTML_TAG_NAVIGATION_PATH
    """ The path of html tags that leads to the desired attribute. """
    attribute_name: str
    """ The name of the tag attribute that should be pulled. """


class EXTRA_ROW_VALUE(TypedDict):
    """ An extra value for the Table_Parser to get from the row and insert into the parsed table. """
    name: str
    """ The name you want the value to have in the table. Can be anything"""
    location: HTML_TAG_VALUE_LOCATION
    """ Where the parser should look for this value. """


class TABLE_ROW_FILTER(TypedDict):
    """ A dictionary that specifies which rows the Table_Parser will filter out. """
    value_location: HTML_TAG_VALUE_LOCATION
    """ The location of the value that should be checked. """
    filtered_values: list[str]
    """ A list of values, which if the parser sees at the given location, will prompt the parser to filter out that row. """

TABLE_COLUMN_FILTER = list[str]
""" A list of the names of columns that should be excluded from the table. """