import SQLite.Constants;
import SQLite.Database;
import SQLite.Exception;
import SQLite.Stmt;

public class testg {
    public static void main(String[] args) throws Exception {
	Database db = new Database();
	db.open(":memory:", Constants.SQLITE_OPEN_READWRITE);
	Stmt createTable = db.prepare("CREATE TABLE test (col1)");
	createTable.step();
	createTable.close();
	Stmt beginTx = db.prepare("BEGIN TRANSACTION");
	beginTx.step();
	beginTx.close();
	for (int i = 0; i < 1000000; i++) {
	    Stmt insert = db.prepare("INSERT INTO test VALUES ('whatever')");
	    insert.step();
	    insert.close();
	}
	Stmt commitTx = db.prepare("COMMIT TRANSACTION");
	commitTx.step();
	commitTx.close();
	db.close();
    }
}
