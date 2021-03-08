import datetime
import sqlite3
import uuid

sql_create_clients_table = """ CREATE TABLE IF NOT EXISTS clients( 
                                ID varchar(16) PRIMARY KEY, 
                                Name varchar(255) , 
                                PublicKey varchar(160) , 
                                LastSeen DATETIME
                                ); """
sql_create_messages_table = """ CREATE TABLE IF NOT EXISTS messages( 
                                ID varchar(4) PRIMARY KEY, 
                                ToClient varchar(16) , 
                                FromClient varchar(16) , 
                                Type varchar(1) , 
                                Content TINYBLOB 
                                ); """


class DataBase:
    def __init__(self):
        try:
            self.connection = sqlite3.connect('server.db')
            self.cursor = self.connection.cursor()
            self.create_data_base()
            sqlite3.register_adapter(uuid.UUID, lambda u: u.bytes)
            self.message_id = 0

        except sqlite3.Error as e:
            print(e)

    def create_table(self, create_table_sql):
        try:
            self.cursor.execute(create_table_sql)
            print(self.cursor.fetchall())
        except sqlite3.Error as e:
            print(e)

    def create_data_base(self):
        if self.connection is not None:
            self.create_table(sql_create_clients_table)  # create clients table
            self.create_table(sql_create_messages_table)  # create messages table
            self.connection.commit()
        else:
            print("Error! cannot create the database connection.")

    def name_exists_in_the_table(self, name):
        self.cursor.execute("SELECT ID, Name FROM clients WHERE Name=?", (name,))
        s = self.cursor.fetchone()
        return True if s else False

    def insert_new_client_to_the_table(self, client_id, name, public_key):
        now = datetime.datetime.now()
        self.cursor.execute("INSERT INTO clients VALUES (?, ?, ?, ?)", (client_id, name, public_key, now))
        self.connection.commit()

    def update_time_for_new_request(self, client_id):
        now = datetime.datetime.now()
        self.cursor.execute("UPDATE clients SET LastSeen=? WHERE ID=?", (now, client_id))
        self.connection.commit()

    def insert_new_message_to_the_table(self, to_client_id, from_client_id, type_m, message):
        self.message_id += 1
        self.cursor.execute("INSERT INTO messages VALUES (?, ?, ?, ?, ?)",
                            (self.message_id, to_client_id, from_client_id, type_m, message))
        self.connection.commit()
        return self.message_id

    def __del__(self):
        self.cursor.close()
        self.connection.close()

    def get_users_list(self, client_id):
        self.cursor.execute(" SELECT ID, Name FROM clients WHERE ID!=? ", (client_id,))
        result = self.cursor.fetchall()
        return result

    def get_public_key(self, client_id):
        self.cursor.execute(" SELECT PublicKey FROM clients WHERE ID=? ", (client_id,))
        result = self.cursor.fetchall()
        return result[0][0]

    def get_messages(self, to_client):
        self.cursor.execute(" SELECT FromClient, ID, Type, Content FROM messages WHERE ToClient=? ", (to_client,))
        result = self.cursor.fetchall()
        return result

    def delete_messages_waiting_to_client(self, client_id):
        self.cursor.execute(" DELETE FROM messages WHERE ToClient=? ", (client_id,))
        self.connection.commit()


db = DataBase()
