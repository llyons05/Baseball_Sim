from bs4 import BeautifulSoup, Comment

import utils

class NoTableFoundException (Exception):

    def __init__(self, *args: object) -> None:
        super().__init__(*args)


class Table_Parser:

    def __init__(self,
                 page_html,
                 table_id: str,
                 table_parent_div_id: str = '') -> None:

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
              cell_specific_data: dict[str, list] = dict(),
              row_filters: list[dict] = []) -> dict[str, list]:

        result: dict[str, list] = dict()

        result['headers'] = self.parse_table_headers(self.table)
        result['data'] = self.parse_table_body(self.table, cell_specific_data, row_filters)

        return result


    def parse_table_headers(self, table: BeautifulSoup) -> list[str]:
        headers: list[str] = []
        table_headers = table.find("thead")
        if table_headers:
            header_cells = table_headers.find_all("th")
            for cell in header_cells:
                headers.append(cell.get("data-stat"))

        return headers


    def parse_table_body(self,
                         table: BeautifulSoup,
                         cell_specific_data: list[str, dict],
                         row_filters: list[dict]) -> list[dict]:

        result: list[dict] = []
        table_rows = table.find("tbody").find_all("tr")

        for row in table_rows:

            if self.is_row_filtered(row, row_filters):
                continue

            row_data = dict()
            row_header = row.find("th")
            if row_header:
                header_cell_name = row_header.get("data-stat")
                row_data[header_cell_name] = self.parse_table_cell(row_header, cell_specific_data.get(header_cell_name, []))

            row_cells = row.findAll("td")
            for cell in row_cells:
                cell_name = cell.get("data-stat")
                row_data[cell_name] = self.parse_table_cell(cell, cell_specific_data.get(cell_name, []))

            result.append(row_data)
        return result


    def parse_table_cell(self, cell: BeautifulSoup, values_to_get: list = []) -> dict:
        result = dict()
        result['text'] = cell.find(string=True)

        hyperlink = cell.find("a")
        if hyperlink:
            result['href'] = hyperlink.get("href")

        for value in values_to_get:
            result[value] = cell.get(value)

        return result


    def is_row_filtered(self, row: BeautifulSoup, row_filters: list[dict]) -> bool:
        for row_filter in row_filters:
            filtered_values = row_filter['filtered_values']
            value = self.get_value_from_tag(row, row_filter.get('interior_tags', []), row_filter['value_name'])
            for filtered_value in filtered_values:
                if (filtered_value == value) or ((value != None) and (filtered_value in value)):
                    return True
    
        return False


    def get_value_from_tag(self,
                           tag: BeautifulSoup,
                           interior_tags: list[str],
                           value_name: str) -> str:

        current_tag = tag
        for interior_tag in interior_tags:
            if current_tag == None:
                return None
            current_tag = current_tag.find(interior_tag)

        if current_tag == None:
            return None

        return current_tag.get(value_name)


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