import subprocess

git_output = subprocess.run(
    ["git", "shortlog", "HEAD", "-sn", "--no-merges"], capture_output=True, text=True
)
lines = git_output.stdout.splitlines()
names = [line.partition("\t")[-1] for line in lines]

text = '<p align="center" style="margin: 0">'
for name in names:
    text = text + name + "<br/>\n"
text = text + "</p>"

file = open("gen_contributors.html", "w")
file.write(text)
file.close()
