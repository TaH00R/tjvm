package tjvm;

public class Main {

    public static void main(String[] args) throws Exception {

        if (args.length != 1) {
            System.out.println("Usage: tjvm <class-file>");
            return;
        }

        ClassReader reader = new ClassReader(args[0]);

        long magic = reader.readU4();

        if (magic != 0xCAFEBABEL) {
            throw new RuntimeException("Not a valid Java class file");
        }

        ClassFile classFile = new ClassFile();

        classFile.magic = magic;
        classFile.minorVersion = reader.readU2();
        classFile.majorVersion = reader.readU2();
        classFile.constantPoolCount = reader.readU2();

        System.out.println("Valid JVM class file!");
        System.out.println();

        classFile.print();

        reader.close();
    }
}