# Deployment

1. Use `cmake` to compile `code` executable, then move it to where flask `run.py` lies. 

2. run

   ```shell
   $ export FLASK_APP=app.py
   $ python -m flask run --host="0.0.0.0" --port=8080
    * Running on http://127.0.0.1:8080/
   ```

   