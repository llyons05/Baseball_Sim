from bs4 import BeautifulSoup, Comment
from typing import Literal, TypedDict

import utils
from table import Table
import table_parsing_types as Extra_Types

class NoTableFoundException (Exception):

    def __init__(self, *args: object) -> None:
        super().__init__(*args)

class Table_Parser:

    def __init__(self,
                 page_html,
                 table_id: str,
                 table_parent_div_id: str = "") -> None:

        self.page_soup = BeautifulSoup(page_html, "html.parser")
        self.table_id = table_id
        self.table_parent_div_id = table_parent_div_id

        if not table_parent_div_id:
            self.table_parent_div_id = "all_" + self.table_id

        self.table = self.extract_table_from_soup(self.page_soup, self.table_id, self.table_parent_div_id)

        if self.table == None:
            raise NoTableFoundException(f"No table with id {table_id} was found in the soup.")


    def extract_table_from_soup(self,
                                 page_soup: BeautifulSoup,
                                 table_id: str,
                                 table_parent_div_id: str) -> BeautifulSoup | None:

        table = page_soup.find("table", {"id": table_id})
        if not table:
            table = self.extract_commented_table(page_soup, table_id, table_parent_div_id)

        return table


    def parse(self,
              extra_row_values: list[Extra_Types.EXTRA_ROW_VALUE] = dict(),
              row_filters: list[Extra_Types.TABLE_ROW_FILTER] = [],
              column_filters: list[Extra_Types.TABLE_COLUMN_FILTER] = [],
              forbidden_chars: dict[str, str] = dict()) -> Table:

        headers = self.parse_table_headers(self.table, column_filters, extra_row_values)
        data = self.parse_table_body(self.table, extra_row_values, row_filters, column_filters)

        return Table(headers, data, forbidden_chars)


    def parse_table_headers(self, table: BeautifulSoup,
                            column_filters: Extra_Types.TABLE_COLUMN_FILTER,
                            extra_columns: list[Extra_Types.EXTRA_ROW_VALUE]) -> list[str]:
        headers: list[str] = []
        table_headers = table.find("thead")
        if table_headers:
            header_cells = table_headers.find_all("th")
            for cell in header_cells:
                value = cell.get("data-stat")
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
        table_rows = table.find("tbody").find_all("tr")

        for row in table_rows:

            if self.is_row_filtered(row, row_filters):
                continue

            row_data: dict[str, str] = dict()
            row_header = row.find("th")
            if row_header:
                header_cell_name = row_header.get("data-stat")
                if (header_cell_name not in column_filters):
                    row_data[header_cell_name] = self.parse_table_cell(row_header)

            row_cells = row.findAll("td")
            for cell in row_cells:
                cell_name = cell.get("data-stat")
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


    def parse_table_cell(self, cell: BeautifulSoup) -> str:
        result = cell.find(string=True)
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

        current_tag = tag
        for navigation_step in value_location.get("tag_navigation_path", []):
            if current_tag == None:
                return None
            current_tag = current_tag.find(navigation_step.get("tag_name"), navigation_step.get("attributes", dict()))

        if current_tag == None:
            return None

        return current_tag.get(value_location["attribute_name"])


    def extract_commented_table(self,
                                 page_soup: BeautifulSoup,
                                 table_id: str,
                                 table_parent_div_id: str) -> BeautifulSoup | None:

        table_div = page_soup.find("div", {"class": "table_wrapper", "id": table_parent_div_id})
        all_comments = self.get_comments(table_div)

        correct_comment = None
        for comment in all_comments:
            if table_id in comment:
                correct_comment = comment
                break

        if not correct_comment:
            return None
        
        comment_soup = BeautifulSoup(str(correct_comment), "html.parser")
        table = comment_soup.find("table", {"id": table_id})
        return table


    def get_comments(self, table_div: BeautifulSoup | None) -> list:
        if table_div is None:
            return []

        return table_div.find_all(string=lambda text: isinstance(text, Comment))