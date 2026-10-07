package tjvm;

import java.io.BufferedInputStream;
import java.io.DataInputStream;
import java.io.FileInputStream;
import java.io.IOException;

public class ClassReader {
    private final DataInputStream in;

    public ClassReader(String file) throws IOException{
        in = new DataInputStream(
                new BufferedInputStream(
                        new FileInputStream(file)
                )
        );
    }

    public int readU1() throws IOException {
        return in.readUnsignedByte();
    }

    public int readU2() throws  IOException {
        return in.readUnsignedShort();
    }

    public long readU4() throws IOException{
        return Integer.toUnsignedLong(in.readInt());
    }

    public byte[] readBytes(int length) throws IOException {
        byte[] bytes = new byte[length];
        in.readFully(bytes);
        return bytes;
    }

    public void close() throws IOException {
        in.close();
    }

}
