The config.json file in this directory is the one I used to render the very garish
colors in the screenshots included in C-Menu's README.md.

You can copy it to your ~/.config/tree-sitter/config.json to use it yourself, or
better yet, modify it to your liking!

You can find documentation on the format of this file in the tree-sitter wiki.

You can also use the "Edit Theme" button in C-Menu to modify the colors
interactively and save your changes to config.json.

My, this is a great idea. I hadn't thought of that! I had planned to switch the
C-Menu configuration to JSON anyway, so this works out nicely. Thanks for the suggestion! Although, I had planned to do that in the Rust version with Serde,
so it might take me a little while to get around to it. Eureka!

I can say the following is remarkably elegant because it isn't my code. Kudos to
the original author!

Here is the content of config.json:

{
  "theme": {
    "background": "#000000",
    "foreground": "#FFFFFF",
    "cursor": "#FF00FF",
    "selection": "#00FFFF",
    "syntax": {
      "comment": "#00FF00",
      "string": "#FFFF00",
      "keyword": "#FF0000",
      "function": "#FFA500",
      "variable": "#800080",
      "number": "#00FFFF",
      "type": "#FFC0CB"
    }
  }
}

