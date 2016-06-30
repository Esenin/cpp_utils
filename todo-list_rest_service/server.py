# python to-do task service
# http://[hostname]/todo/api/v1/tasks
# task : id, title, descr, posted date, done

from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
from SocketServer import ThreadingMixIn
import threading
import argparse
import cgi
import logging
import mysql.connector
import sys
import json
from datetime import date, datetime


class MyHttpHandler(BaseHTTPRequestHandler):
	db_cnx = None
	def do_GET(self):
		RESPONSE_OK = 200
		T_ID = threading.currentThread().getName()
		response = RESPONSE_OK
		logging.info("-----GET request accepted from {}, path= {}".format(self.client_address, self.path) + 40 * "-")
		logging.info(T_ID)
		logging.info("RAW: {}".format(self.raw_requestline) + " <<< EOF\n")
		url_parts = self.path[1:].split('/')

		query = url_parts[4] if len(url_parts) > 4 else ""
		if url_parts[0:4] != ["todo", "api", "v1", "tasks"]:
			response = 404
		elif not query.isdigit() and query != "*":
			response = 403

		self.send_response(response)
		mimetype = "application/json"
		self.send_header('Content-type', mimetype)
		self.end_headers()
		if response != RESPONSE_OK:
			logging.info("bad response: " + str(response))
			return

		cursor = self.db_cnx.cursor()
		db_query = "SELECT title, description, posted_date, done FROM todo_tb "
		if query.isdigit():
			db_query = db_query + "WHERE id = {}".format(query)

		logging.info("db_query: " + db_query)
		cursor.execute(db_query)
		response_body = []
		for (title, description, posted_date, done) in cursor:
			response_body.append({"title" : title, "description" : description, "posted_date" : "{:%d %b %Y}".format(
				posted_date), "done" : done})
		cursor.close()
		response_body = json.dumps(response_body)
		logging.info("Server answer : " + response_body)
		self.wfile.write(response_body)

		return

	def parse_POST(self):
		ctype, pdict = cgi.parse_header(self.headers['content-type'])
		if ctype == 'application/json':
			length = int(self.headers['content-length'])
			postvars = cgi.parse_qs(self.rfile.read(length), keep_blank_values=1)
		else:
			postvars = {}
		return postvars.keys()[0] if len(postvars) > 0 else ""


	def do_POST(self):
		RESPONSE_OK = 202
		response = RESPONSE_OK
		T_ID = threading.currentThread().getName()
		logging.info("-----POST request accepted from {}, path= {}".format(self.client_address, self.path) + 40 * "-")
		logging.info(T_ID)
		logging.info("RAW: {}".format(self.raw_requestline) + " <<< EOF\n")

		new_data = json.loads(self.parse_POST())
		logging.info("json = " + str(new_data))

		if new_data["title"] == "" or new_data["description"] == "":
			response = 412
		if len(new_data["title"]) > 200:
			response = 413

		self.send_response(response)
		mimetype = "application/json"
		self.send_header('Content-type',mimetype)
		self.end_headers()
		if response != RESPONSE_OK:
			logging.info("bad response: " + str(response))
			return

		cur_date = str(datetime.now().date())
		cursor = self.db_cnx.cursor()
		db_query = "INSERT INTO todo_tb (title, description, posted_date, done)  VALUES ('{}', '{}', '{}', 0)".format(
			new_data["title"], new_data["description"], cur_date)

		logging.info("db_query: " + db_query)
		cursor.execute(db_query)
		self.db_cnx.commit()
		cursor.close()


class ThreadedHTTPServer(ThreadingMixIn, HTTPServer):
    """Handle requests in a separate thread."""

if __name__=='__main__':
	logging.basicConfig(format='%(asctime)s %(message)s', level=logging.DEBUG)

	parser = argparse.ArgumentParser(description='HTTP Server')
	parser.add_argument('port', type=int, help='Listening port for HTTP Server')
	args = parser.parse_args()

	try:
		cnx = mysql.connector.connect(user='todo_client', password='td_cli_pass7', host='localhost',
		                              database='TODOLIST')
		MyHttpHandler.db_cnx = cnx
		logging.info("Connection to DB established")
	except:
		logging.error("ERR: Cannot connect to DB. exit")
		sys.exit("DB error")

	logging.info("starting server...")
	try:
		httpd =  ThreadedHTTPServer(('localhost', args.port), MyHttpHandler)
		logging.info ('HTTP Server has been started at 0.0.0.0:{}'.format(args.port))
		httpd.serve_forever()
	except KeyboardInterrupt:
		logging.info("^C received, shutting down the web server")
		httpd.socket.close()
	except:
		logging.error("ERR: Unable to startup http server")
	MyHttpHandler.db_cnx.close()
