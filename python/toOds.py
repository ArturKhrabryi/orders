import argparse
from pathlib import Path
from typing import Iterable
from Database import Product, Database

from odf.opendocument import OpenDocumentSpreadsheet
from odf.table import Table, TableRow, TableCell, TableColumn
from odf.text import P
from odf.style import (
   Style,
   TableColumnProperties,
   TableCellProperties,
   ParagraphProperties,
   PageLayout,
   PageLayoutProperties,
   MasterPage
)


class OdsExporter:
    COL_WIDTHS_CM: tuple[float, ...] = (12.5, 3.0, 1.3, 2.0)
    HEADER_TITLES: tuple[str, ...] = ("Nazwa", "Kod kreskowy", "Ilość", "Jednostka")

    def __init__(self) -> None:
        self.doc = OpenDocumentSpreadsheet()
        self._register_page_styles()
        self._register_cell_styles()


    def _register_page_styles(self) -> None:
        page_layout = PageLayout(name="layout")
        page_layout.addElement(PageLayoutProperties(margin="1cm", tablecentering="horizontal"))
        master_page = MasterPage(name="Default", pagelayoutname=page_layout)
        self.doc.automaticstyles.addElement(page_layout)
        self.doc.masterstyles.addElement(master_page)


    def _cell_style(self, name: str, bg: str, align: str = "center") -> Style:
        style = Style(name=name, family="table-cell")
        style.addElement(ParagraphProperties(textalign=align))
        style.addElement(TableCellProperties(backgroundcolor=bg, border="0.05pt solid #000000"))
        self.doc.automaticstyles.addElement(style)

        return style


    def _register_cell_styles(self) -> None:
        self.styles: dict[str, Style] = {
            "white": self._cell_style("cellwhite", "#ffffff"),
            "grey": self._cell_style("cellgrey", "#dddddd"),
            "white_left": self._cell_style("cellwhiteleft", "#ffffff", "left"),
            "grey_left": self._cell_style("cellgreyleft", "#dddddd", "left")
        }


    def _add_columns(self, table: Table) -> None:
        for idx, width in enumerate(self.COL_WIDTHS_CM):
            col_style = Style(name=f"col{idx}", family="table-column")
            col_style.addElement(TableColumnProperties(columnwidth=f"{width}cm"))
            self.doc.automaticstyles.addElement(col_style)
            table.addElement(TableColumn(stylename=col_style))


    def build_table(self, products: Iterable[Product]) -> None:
        table = Table(name="Zamówienie")
        self._add_columns(table)

        header_row = TableRow()
        for title in self.HEADER_TITLES:
            cell = TableCell(stylename=self.styles["white"])
            cell.addElement(P(text=title))
            header_row.addElement(cell)

        table.addElement(header_row)

        for idx, product in enumerate(products):
            if product.code_ean is None:
                product.code_ean = "-"

            shade = "white" if idx % 2 else "grey"
            row = TableRow()

            first_cell = TableCell(stylename=self.styles[f"{shade}_left"])
            first_cell.addElement(P(text=product.name))
            row.addElement(first_cell)

            for value in (product.code_ean, product.quantity, product.unit_code):
                cell = TableCell(stylename=self.styles[shade])
                if isinstance(value, float):
                    value_str = str(int(value)) if value.is_integer() else str(value)

                else:
                    value_str = str(value)
                    
                cell.addElement(P(text=value_str))
                row.addElement(cell)

            table.addElement(row)

        self.doc.spreadsheet.addElement(table)


    def save(self, outfile: Path) -> None:
        if outfile.suffix.lower() != ".ods":
            outfile = outfile.with_suffix(".ods")

        self.doc.save(outfile)
        print("ODS written to ", outfile)


def parse_args() -> Path:
    parser = argparse.ArgumentParser(
        prog="toOds",
        description="Create an .ods file from an SQLite `products` table."
    )
    parser.add_argument("database", type=Path, help="Path to the SQLite DB file")
    
    return parser.parse_args().database


def main() -> None:
    db_path = parse_args()

    with Database(path=db_path) as db:
        products = db.fetch_products()
        if not products:
            print("Table `products` is empty - nothing to export")
        
            return

        exporter = OdsExporter()
        exporter.build_table(products)
        exporter.save(db_path)

        print("Done")


if __name__ == "__main__":
    main()
