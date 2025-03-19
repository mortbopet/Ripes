from flask import Flask, render_template, request

app = Flask(
    __name__,
    template_folder="templates",
    static_folder="static"
)


@app.route('/', methods=['GET', 'POST'])
def main_page() -> str:
    if request.method == 'POST':
        print('пришел POST')
        return render_template("index.html")
    else:
        print('пришел GET')
        return render_template("index.html")


if __name__ == '__main__':
    app.run(debug=True)
