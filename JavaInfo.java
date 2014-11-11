public class JavaInfo {
	public JavaInfo() {
		String strHome = System.getProperty("java.home");
		if (strHome.endsWith( "/" + "jre" )) {
			strHome = strHome.substring(0, strHome.length()-4);
		}

		System.out.println(strHome);
	}

	public static void main(String args[]) {
		new JavaInfo();
	}
}

