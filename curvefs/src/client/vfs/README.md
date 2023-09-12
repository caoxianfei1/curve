
User Guide
===

Permission
---

`uid/gid` AND `euid/egid`

```C++
euid = uid
egid = gid

if (suid != NULL) {
    euid = suid;
}

if (sgid != NULL) {
    egid = sgid;
}
```

