package io.opencurve.curve.fs.libfs;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.Map;
import java.lang.System;
import java.nio.ByteBuffer;

public class Mapping {
    public static final String PASSWD_FILE = "/etc/passwd";
    public static final String GROUP_FILE = "/etc/group";

    private String salt;      
    private boolean local;     

    private Map<String, Integer> usernames;
    private Map<Integer, String> userIDs;
    private Map<String, Integer> groups;
    private Map<Integer, String> groupIDs;

    private static ByteBuffer buffer = ByteBuffer.allocate(Long.BYTES); 

    public Mapping(String salt) throws IOException {
        this.salt = salt;
        usernames = new HashMap<String, Integer>();
        userIDs = new HashMap<Integer, String>();
        groups = new HashMap<String, Integer>();
        groupIDs = new HashMap<Integer, String>();
        this.update(genAllUids(), genAllGids(), true);
    }
    
    private int getGuidFromLocal(String name, String path) throws IOException {
        int id = -1;
        try {
            BufferedReader reader = new BufferedReader(new FileReader(path));
            String line;
            while ((line = reader.readLine()) != null) {
                String[] fields = line.split(":");
                if (fields[0].equals(name)) {
                    id = Integer.parseInt(fields[2]);
                    break;
                }
            }
            reader.close();
        } catch (IOException e) {
            throw new IOException("failed to read" + path);
        }
        return id;
    }

    private String getGuNameFromLocal(int id, String path) throws IOException {
        String name = null;
        try {
            BufferedReader reader = new BufferedReader(new FileReader(path));
            String line;
            while ((line = reader.readLine()) != null) {
                String[] fields = line.split(":");
                if (fields[2].equals(String.valueOf(id))) {
                    name = fields[2];
                    break;
                }
            }
            reader.close();
        } catch (IOException e) {
            throw new IOException("failed to read" + path);
        }
        return name;
    }

    private Map<String, Integer> genAllGuids(String path) throws IOException {
        Map<String, Integer> name2Ids = new HashMap<String, Integer>();
        try {
            BufferedReader reader = new BufferedReader(new FileReader(path));
            String line;
            while ((line = reader.readLine()) != null) {
                String[] fields = line.split(":");
                if (fields.length < 3) {
                    continue;
                }
                name2Ids.put(fields[0], Integer.parseInt(fields[2]));
            }
            reader.close();
        } catch (IOException e) {
            throw new IOException("failed to read" + path);
        }
        return name2Ids;
    }

    private int genGuid(String name) throws UnsupportedEncodingException, NoSuchAlgorithmException {
        String str = salt + name + salt;
        byte[] bytesOfMessage = str.getBytes("UTF-8");
        MessageDigest md = MessageDigest.getInstance("MD5");
        byte[] md5Digest = md.digest(bytesOfMessage);
        
        int length = 8;
        byte[] b1 = new byte[length];
        byte[] b2 = new byte[length];
        System.arraycopy(md5Digest, 0, b1, 0, length);
        System.arraycopy(md5Digest, 8, b2, 0, length);

        buffer.put(b1, 0, b1.length);
        buffer.flip();
        long a = buffer.getLong();
        
        buffer.put(b2, 0, b2.length);
        buffer.flip();
        long b = buffer.getLong();

        return (int)(a ^ b);
    }

    public Map<String, Integer> genAllUids() throws IOException {
        return genAllGuids(PASSWD_FILE);
    }

    public Map<String, Integer> genAllGids() throws IOException {
        return genAllGuids(GROUP_FILE);
    }

    public void update(Map<String, Integer> user2Ids, Map<String, Integer> group2Ids, boolean local) {
        this.local = local;
        for (Map.Entry<String, Integer> entry : user2Ids.entrySet()) {
            String userName = entry.getKey();
            int userId = entry.getValue();
			int oldId = usernames.get(userName);
            String oldName = userIDs.get(userId);
            usernames.remove(oldName);
            userIDs.remove(oldId);
            usernames.put(userName, userId);
            userIDs.put(userId, userName);
		}
        for (Map.Entry<String, Integer> entry : group2Ids.entrySet()) {
            String groupName = entry.getKey();
            int groupId = entry.getValue();
			int oldId = groups.get(groupName);
            String oldName = groupIDs.get(groupId);
            groups.remove(oldName);
            groupIDs.remove(oldId);
            groups.put(groupName, groupId);
            groupIDs.put(groupId, groupName);
		}
    }

    public int lookupUser(String user) throws IOException, NoSuchAlgorithmException {
        int id;
        if (usernames.containsKey(user)) {
            return usernames.get(user);
        } 
        
        if (!local) {
            id = genGuid(user);
            usernames.put(user, id);
            userIDs.put(id, user);
            return id;
        }

        if (user.equals("root")) {
            id = genGuid(user); // root in hdfs sdk is a normal user
        } else {
            int uid = getGuidFromLocal(user, PASSWD_FILE);
            if (uid < 0) {
                id = genGuid(user);
            } else {
                id = uid;
            }
        }

        usernames.put(user, id);
        userIDs.put(id, user);
        return id;
    }

    public int lookupGroup(String group) throws IOException, NoSuchAlgorithmException {
        int id;
        if (groups.containsKey(group)) {
            return groups.get(group);
        } 
        
        if (!local) {
            id = genGuid(group);
            groups.put(group, id);
            groupIDs.put(id, group);
            return id;
        }

        if (group.equals("root")) {
            id = genGuid(group);
        } else {
            int gid = getGuidFromLocal(group, GROUP_FILE);
            if (gid < 0) {
                id = genGuid(group);
            } else {
                id = gid;
            }
        }

        groups.put(group, id);
        groupIDs.put(id, group);
        return id;
    }

    public String lookupUserID(int uid) throws IOException {
        String username = null;
        if (userIDs.containsKey(uid)) {
            return userIDs.get(uid);
        } 
        
        if (!local) {
            return String.valueOf(uid);
        }
        
        String name = getGuNameFromLocal(uid, PASSWD_FILE);
        if (name != null && !"".equals(name)) {
            username = name;
            if (name.length() > 49) {
                username = name.substring(0, 49);
            }
        } else {
            username = String.valueOf(uid);
        }
        usernames.put(username, uid);
        userIDs.put(uid, username);
        return username;
    }

    public String lookupGroupID(int gid) throws IOException {
        String groupname = null;
        if (groupIDs.containsKey(gid)) {
            return groupIDs.get(gid);
        } 
        
        if (!local) {
            return String.valueOf(gid);
        }
        
        String name = getGuNameFromLocal(gid, GROUP_FILE);
        if (name != null && !"".equals(name)) {
            groupname = name;
            if (name.length() > 49) {
                groupname = name.substring(0, 49);
            }
        } else {
            groupname = String.valueOf(gid);
        }

        groups.put(groupname, gid);
        groupIDs.put(gid, groupname);
        return groupname;
    }
}
