from __future__ import annotations

import sqlite3
from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional, Self


def to_number(num: float) -> int | float:
    if num.is_integer():
        return int(num)

    return num


def from_str_to_number(text: str) -> int | float:
    s = text.strip().replace(",", ".")
    num = float(s)

    return to_number(num)


@dataclass(slots=True, kw_only=True)
class Product:
    name: str
    code_ean: Optional[str]
    quantity: int | float
    unit_code: str

    def __str__(self) -> str:
        return f"{self.name}  {self.code_ean or '-'}  {self.quantity}  {self.unit_code}"

    @classmethod
    def from_row(cls, row: sqlite3.Row) -> Product:
        return cls(name=row["name"], code_ean=row["codeEan"], quantity=to_number(row["quantity"]), unit_code=row["unitCode"])

    @classmethod
    def from_text(cls, name: str, code_ean: Optional[str], quantity: str, unit_code: str) -> Product:
        return cls(name=name, code_ean=code_ean, quantity=from_str_to_number(quantity), unit_code=unit_code)

class Database:
    def __init__(self, path: str | Path, *, detect_types: int = 0, isolation_level: Optional[str] = None, timeout: float = 5.0) -> None:
        self.path = str(path)
        self.detect_types = detect_types
        self.isolation_level = isolation_level
        self.timeout = timeout

        self._conn: Optional[sqlite3.Connection] = None
        self._cur: Optional[sqlite3.Cursor] = None

    def connect(self) -> None:
        if self._conn is not None:
            return
        
        self._conn = sqlite3.connect(self.path, detect_types=self.detect_types, isolation_level=self.isolation_level, timeout=self.timeout)
        self._conn.row_factory = sqlite3.Row
        self._cur = self._conn.cursor()

    def close(self) -> None:
        if self._cur is not None:
            self._cur.close()
        if self._conn is not None:
            self._conn.close()
        self._cur = self._conn = None

    def __enter__(self) -> Self:
        self.connect()

        return self


    def __exit__(self, exc_type, exc, tb) -> None:
        self.close()

    def _ensure(self) -> tuple[sqlite3.Connection, sqlite3.Cursor]:
        if self._conn is None or self._cur is None:
            raise RuntimeError("Database is not connected. Use connect() or a context manager.")

        return self._conn, self._cur
    
    def fetch_products(self) -> list[Product]:
        _, cur = self._ensure()
        cur.execute("SELECT name, codeEan, quantity, unitCode FROM orderLines JOIN products ON orderLines.productId = products.id ORDER BY orderLines.id")

        return [Product.from_row(r) for r in cur.fetchall()]

