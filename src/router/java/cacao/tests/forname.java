class forname {
    public static void main(String[] argv) {
        new forname_init();
    }
}

class forname_init {
    static {
        System.out.println("<clinit>");
    }

    forname_init() {
        System.out.println("<init>");
    }
}
