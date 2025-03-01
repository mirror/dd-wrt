package SQLite;

/**
 * Callback interface for SQLite's user defined progress handler.
 */

public interface ProgressHandler {

    /**
     * Invoked for N SQLite VM opcodes.
     * The method should return true to continue the
     * current query, or false in order
     * to abandon the action.<BR><BR>
     *
     * @return true to continue, false else
     */

    public boolean progress();
}
