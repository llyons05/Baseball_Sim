class Table:

    def __init__(self, headers: list[str] = [], rows_of_data: list[dict[str, str]] = [], forbidden_characters: dict[str, str] = dict()) -> None:
        self.headers: list[str] = []
        self.rows: list[dict[str, str]] = []
        self.forbidden_chars = forbidden_characters

        self.add_headers(headers)
        self.add_rows(rows_of_data)


    def add_headers(self, headers: list[str]) -> None:
        for header in headers:
            self.add_header(header)


    def add_header(self, header: str) -> None:
        if header not in self.headers:
            self.headers.append(self.filter_forbidden_chars(header))


    def add_rows(self, rows: list[dict[str, str]], inherit_new_headers: bool = False) -> None:
        for row in rows:
            self.add_row(row, inherit_new_headers)


    def add_row(self, row: dict[str, str], inherit_new_headers: bool = False) -> None:
        filtered_row: dict[str, str] = dict()

        if inherit_new_headers:
            self.add_headers(row.keys())

        for header in self.headers:
            filtered_row[header] = self.filter_forbidden_chars(row.get(header))
        
        self.rows.append(filtered_row)


    def search_rows(self, search_attributes: dict[str, str]) -> list[dict[str, str]]:
        result = []
        for row in self.rows:
            meets_search_criteria = True
            for key in search_attributes.keys():
                if row[key] != search_attributes[key]:
                    meets_search_criteria = False
                    break
            if meets_search_criteria:
                result.append(row)
        return result


    def sort(self, key, reversed: bool = False):
        self.rows.sort(key=key, reverse=reversed)


    def filter_forbidden_chars(self, input_str: str) -> str:
        if not input_str:
            return input_str

        for forbidden_char in self.forbidden_chars.keys():
            replacement_char = self.forbidden_chars[forbidden_char]
            input_str = input_str.replace(forbidden_char, replacement_char)
        return input_str


    def add_forbidden_char(self, forbidden_char: str, replacement_char: str, filter_previously_existing_values: bool = False) -> None:
        self.forbidden_chars[forbidden_char] = replacement_char

        if filter_previously_existing_values:
            temp_headers = self.headers.copy()
            self.headers = []
            self.add_headers(temp_headers)

            temp_rows = self.rows.copy()
            self.rows = []
            self.add_rows(temp_rows)

    def __iter__(self):
        return iter(self.rows)


    def __len__(self) -> int:
        return len(self.rows)


EMPTY_TABLE = Table()