# wordle
Provide a list of candidate words for use in Wordle, given a set of restrictions on allowed and disallowed letters and placements.

## Usage
```
wordle <required_arg> <forbidden_arg> <placement_arg>
    required_arg: 5 character string specifying locations of required characters, - for blanks. Ex: "br-a-"
    forbidden_arg: list of characters forbidden in the final word. Ex: "qyz"
    placement_arg: <character><disallowed location><disallowed location>, 0-indexed. Ex: "e03d123"
```