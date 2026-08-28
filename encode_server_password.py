#!/usr/bin/env python3
"""Encode and decode 32-character Password values used by Bishop/Goddess.

This implements the inverse of the server's SimplyDecryptPassword/pgDecrypt
routine.  It is compatibility obfuscation, not modern password encryption.
"""

import argparse
import secrets
import string


# Keep generated config values convenient to paste into INI files and shells.
# The server accepts all printable ASCII, but new values deliberately use only
# letters, digits, and a small, readable set of punctuation.  `>` is needed
# solely for the encoded length of a 20-character password.
PRINTABLE = "".join(chr(value) for value in range(0x20, 0x7F))
OUTPUT_ALPHABET = string.ascii_letters + string.digits + "_[]`>!$"
SWAPS = ((0, 13), (31, 25), (12, 30), (7, 19), (3, 21), (9, 20), (15, 18))


def pg_char2int(char: str) -> int:
    value = ord(char) ^ 0x97
    return ((value << 7) | (value >> 1)) & 0x1F


def encode_number(value: int, alphabet: str = PRINTABLE) -> str:
    for char in alphabet:
        if pg_char2int(char) == value:
            return char
    raise ValueError(f"Cannot encode value {value}")


def swap_chars(value: list[str]) -> None:
    for left, right in SWAPS:
        value[left], value[right] = value[right], value[left]


def encrypt(password: str) -> str:
    """Return a 32-char value for Password= (plaintext length: 0..20)."""
    if not 0 <= len(password) <= 20:
        raise ValueError("Password must contain from 0 to 20 ASCII characters")
    if any(char not in PRINTABLE for char in password):
        raise ValueError("Password must use printable ASCII characters only")

    # The original format has a random key length (10..30) and a random
    # printable key.  Select a length that leaves space for its ciphertext.
    # Limiting the key to 10..19 means its encoded length always has a
    # representation in OUTPUT_ALPHABET.
    key_length = secrets.randbelow(min(19, 30 - len(password)) - 9) + 10
    key_chars = []
    for key_index in range(key_length):
        # A key byte can be reused by the format. Pick one that produces an
        # allowed ciphertext byte for every plaintext character using it.
        candidates = [
            key_char for key_char in OUTPUT_ALPHABET
            if all(
                chr((ord(password[index]) - 0x20 - 0x3F + ord(key_char) - 0x20) % 0x5F)
                in OUTPUT_ALPHABET
                for index in range(key_index, len(password), key_length)
            )
        ]
        if not candidates:
            raise ValueError("Password cannot be encoded with the restricted output alphabet")
        key_chars.append(secrets.choice(candidates))
    ciphertext_chars = [
        chr((ord(plain) - 0x20 - 0x3F + ord(key_chars[index % key_length]) - 0x20) % 0x5F)
        for index, plain in enumerate(password)
    ]
    key = "".join(key_chars)
    ciphertext = "".join(ciphertext_chars)

    # Before the fixed permutation, format is: length, key, password length,
    # ciphertext, then irrelevant printable padding.
    raw = list(
        encode_number(key_length, OUTPUT_ALPHABET)
        + key
        + encode_number(len(password), OUTPUT_ALPHABET)
        + ciphertext
    )
    raw.extend(secrets.choice(OUTPUT_ALPHABET) for _ in range(32 - len(raw)))
    swap_chars(raw)  # Same swaps reverse the server's swap operation.
    return "".join(raw)


def decrypt(value: str) -> str:
    """Local verification implementation of the server's decrypt routine."""
    if len(value) != 32:
        raise ValueError("Encoded value must have exactly 32 characters")
    raw = list(value)
    swap_chars(raw)
    key_length = pg_char2int(raw[0])
    password_length = pg_char2int(raw[key_length + 1])
    if not 10 <= key_length <= 30 or not 0 <= password_length <= 20:
        raise ValueError("Invalid server password value")
    key = raw[1 : key_length + 1]
    ciphertext = raw[key_length + 2 : key_length + 2 + password_length]
    return "".join(
        chr(((ord(cipher) + 0x3F - (ord(key[index % key_length]) - 0x20)) % 0x5F) + 0x20)
        for index, cipher in enumerate(ciphertext)
    )


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Encode/decode Bishop/Goddess Password values (custom obfuscation)"
    )
    commands = parser.add_subparsers(dest="command", required=True)
    encode_parser = commands.add_parser("encode", help="make a 32-character config value")
    encode_parser.add_argument("password", help="plaintext password (0..20 printable ASCII characters)")
    decode_parser = commands.add_parser("decode", help="read a 32-character config value")
    decode_parser.add_argument(
        "value",
        nargs="?",
        help="32-character value after Password = (omit to paste it interactively)",
    )
    args = parser.parse_args()
    try:
        if args.command == "encode":
            encoded = encrypt(args.password)
            assert decrypt(encoded) == args.password
            print(encoded)
        else:
            value = args.value if args.value is not None else input("Dán chuỗi 32 ký tự rồi nhấn Enter: ")
            print(decrypt(value))
    except ValueError as error:
        parser.error(str(error))


if __name__ == "__main__":
    main()
