from bs4 import BeautifulSoup, Comment
from typing import Literal, TypedDict
from collections.abc import Callable

import utils
from table import Table
import table_parsing_types as Extra_Types

class NoTableFoundException (Exception):

    def __init__(self, *args: object) -> None:
        super().__init__(*args)

def default_parse_table_cell(cell: BeautifulSoup) -> str:
    result = cell.find(string=True)
    return result

class Table_Parser:

    def __init__(self,
                 page_html,
                 table_location: Extra_Types.HTML_TAG_NAVIGATION_PATH,
                 table_wrapper_div_location: Extra_Types.HTML_TAG_NAVIGATION_PATH,
                 table_header_tag_name: str = "thead",
                 table_body_tag_name: str = "tbody",
                 table_row_tag_name: str = "tr",
                 row_header_cell_tag_name: str = "th",
                 row_cell_tag_name: str = "td",
                 cell_descriptor_attribute_name: str = "data-stat",
                 cell_parsing_method: Callable[[BeautifulSoup], str] = default_parse_table_cell) -> None:

        self.page_soup = BeautifulSoup(page_html, "html.parser")
        self.table_location = table_location
        self.table_wrapper_div_location = table_wrapper_div_location

        self.table = self.extract_table_from_soup(self.page_soup, self.table_location, self.table_wrapper_div_location)

        if self.table == None:
            raise NoTableFoundException(f"No table at location {table_location} was found in the soup.")

        self.table_row_tag_name = table_row_tag_name
        self.table_header_tag_name = table_header_tag_name
        self.table_body_tag_name = table_body_tag_name
        self.row_header_cell_tag_name = row_header_cell_tag_name
        self.row_cell_tag_name = row_cell_tag_name
        self.cell_descriptor_attribute_name = cell_descriptor_attribute_name

        self.parse_table_cell = cell_parsing_method


    def extract_table_from_soup(self,
                                 page_soup: BeautifulSoup,
                                 table_location: Extra_Types.HTML_TAG_NAVIGATION_PATH,
                                 table_wrapper_div_location: Extra_Types.HTML_TAG_NAVIGATION_PATH) -> BeautifulSoup | None:

        table = self.get_tag_from_soup(page_soup, table_location)
        if not table:
            table = self.extract_commented_table(page_soup, table_location, table_wrapper_div_location)

        return table


    def parse(self,
              extra_row_values: list[Extra_Types.EXTRA_ROW_VALUE] = dict(),
              row_filters: list[Extra_Types.TABLE_ROW_FILTER] = [],
              column_filters: list[Extra_Types.TABLE_COLUMN_FILTER] = [],
              forbidden_chars: dict[str, str] = dict()) -> Table:

        headers = self.parse_table_headers(self.table, column_filters, extra_row_values)
        data = self.parse_table_body(self.table, extra_row_values, row_filters, column_filters)

        result = Table(headers, forbidden_characters=forbidden_chars)
        result.add_rows(data, True)

        return result


    def parse_table_headers(self, table: BeautifulSoup,
                            column_filters: Extra_Types.TABLE_COLUMN_FILTER,
                            extra_columns: list[Extra_Types.EXTRA_ROW_VALUE]) -> list[str]:
        headers: list[str] = []
        table_headers = table.find(self.table_header_tag_name)
        if table_headers:
            header_cells = table_headers.find_all(self.row_header_cell_tag_name)
            for cell in header_cells:
                value = cell.get(self.cell_descriptor_attribute_name)
                if value not in column_filters:
                    headers.append(value)

        headers.extend(extra_column["name"] for extra_column in extra_columns)
        return headers


    def parse_table_body(self,
                         table: BeautifulSoup,
                         extra_row_values: list[Extra_Types.EXTRA_ROW_VALUE],
                         row_filters: list[Extra_Types.TABLE_ROW_FILTER],
                         column_filters: Extra_Types.TABLE_COLUMN_FILTER) -> list[dict[str, str]]:

        result: list[dict[str, str]] = []
        table_rows = table.find(self.table_body_tag_name).find_all(self.table_row_tag_name)

        for row in table_rows:

            if self.is_row_filtered(row, row_filters):
                continue

            row_data: dict[str, str] = dict()
            row_header = row.find(self.row_header_cell_tag_name)
            if row_header:
                header_cell_name = row_header.get(self.cell_descriptor_attribute_name)
                if (header_cell_name not in column_filters):
                    row_data[header_cell_name] = self.parse_table_cell(row_header)

            row_cells = row.findAll(self.row_cell_tag_name)
            for cell in row_cells:
                cell_name = cell.get(self.cell_descriptor_attribute_name)
                if cell_name and (cell_name not in column_filters):
                    row_data[cell_name] = self.parse_table_cell(cell)

            extra_values = self.get_extra_row_values(row, extra_row_values)
            row_data.update(extra_values)

            result.append(row_data)

        return result


    def get_extra_row_values(self, row: BeautifulSoup, extra_values: list[Extra_Types.EXTRA_ROW_VALUE]) -> dict[str, str]:
        result: dict[str, str] = dict()
        for extra_value in extra_values:
            key = extra_value["name"]
            value = self.get_value_from_tag(row, extra_value["location"])
            if value == None:
                value = ""
            result[key] = value
        
        return result


    def is_row_filtered(self, row: BeautifulSoup, row_filters: list[Extra_Types.TABLE_ROW_FILTER]) -> bool:
        for row_filter in row_filters:
            filtered_values = row_filter['filtered_values']
            value = self.get_value_from_tag(row, row_filter["value_location"])
            for filtered_value in filtered_values:
                if (filtered_value == value) or ((value != None) and (filtered_value in value)):
                    return True
        return False


    def get_value_from_tag(self, tag: BeautifulSoup, value_location: Extra_Types.HTML_TAG_VALUE_LOCATION) -> str:
        current_tag = self.get_tag_from_soup(tag, value_location.get("tag_navigation_path", []))

        if current_tag == None:
            return None

        if (value_location.get("attribute_name")):
            return current_tag.get(value_location["attribute_name"])
        else:
            return current_tag.find(string=True)


    def get_tag_from_soup(self, soup: BeautifulSoup, tag_location: Extra_Types.HTML_TAG_NAVIGATION_PATH) -> BeautifulSoup:
        current_tag = soup
        for navigation_step in tag_location:
            if current_tag == None:
                return None
            current_tag = current_tag.find(navigation_step.get("tag_name"), navigation_step.get("attributes", dict()))
        
        return current_tag


    def extract_commented_table(self,
                                 page_soup: BeautifulSoup,
                                 table_location: Extra_Types.HTML_TAG_NAVIGATION_PATH,
                                 table_wrapper_div_location: Extra_Types.HTML_TAG_NAVIGATION_PATH) -> BeautifulSoup | None:

        table_div = self.get_tag_from_soup(page_soup, table_wrapper_div_location)
        all_comments = self.get_comments(table_div)

        correct_comment = None
        for comment in all_comments:
            if "table" in comment:
                correct_comment = comment
                break

        if not correct_comment:
            return None

        comment_soup = BeautifulSoup(str(correct_comment), "html.parser")
        table = self.get_tag_from_soup(comment_soup, [table_location[-1]])
        return table


    def get_comments(self, table_div: BeautifulSoup | None) -> list:
        if table_div is None:
            return []

        return table_div.find_all(string=lambda text: isinstance(text, Comment))