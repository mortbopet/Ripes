import subprocess

git_output = subprocess.run(
    ["git", "shortlog", "-sn", "--no-merges"], capture_output=True, text=True
)
lines = git_output.stdout.splitlines()
names = [line.partition("\t")[-1] for line in lines]

text = ""
for name in names:
    text = text + "<p align=\"center\" style=\"margin: 0\">" + name + "</p>\n"

file = open("gen_contributors.html", "w")
file.write(text)
file.close()
