package tjvm;

public class ClassFile {
    public long magic;
    public int minorVersion;
    public int majorVersion;
    public int constantPoolCount;
    public int accessFlags;
    public int thisClass;
    public int superClass;

    public void print(){
        System.out.printf("Magic:           0x%08X%n", magic);
        System.out.println("Minor version:   " + minorVersion);
        System.out.println("Major version:   " + majorVersion);
        System.out.println("Constant pool:   " + constantPoolCount);
        System.out.printf("Access flags:    0x%04X%n", accessFlags);
        System.out.println("This class:      #" + thisClass);
        System.out.println("Super class:     #" + superClass);
    }
}
