## Delete Shadow Copies Using The [IOCTL_VOLSNAP_DELETE_SNAPSHOT](https://github.com/NUL0x4C/IOCTL_VOLSNAP_DELETE_SNAPSHOT/blob/main/IOCTL_VOLSNAP_DELETE_SNAPSHOT/Main.cpp#L17) IOCTL

</br>

> [!NOTE]  
> **This branch uses the VSS APIs for Shadow Copies enumeration. To implement IOCTL purely, refer to the newer [branch](https://github.com/NUL0x4C/IOCTL_VOLSNAP_DELETE_SNAPSHOT/tree/IOCTL_VOLSNAP_QUERY_NAMES_OF_SNAPSHOTS), utilizing `IOCTL_VOLSNAP_QUERY_NAMES_OF_SNAPSHOTS`**.

</br>

> [!NOTE]  
> **Another way to avoid using VSS is to "brute-force" the Shadow Copies' Volumes for every device (no enumeration required). This is done [here](https://github.com/NUL0x4C/PSBits/blob/master/IOCTL_VOLSNAP_DELETE_SNAPSHOT/IOCTL_VOLSNAP_DELETE_SNAPSHOT.c)**

</br>

<img width="1190" height="749" alt="image" src="https://github.com/user-attachments/assets/90c67e69-6078-4663-9faa-f1d5a99288cd" />



</br>

### This repository is inspired by [gtworek/IOCTL_VOLSNAP_SET_MAX_DIFF_AREA_SIZE](https://github.com/gtworek/PSBits/tree/master/IOCTL_VOLSNAP_SET_MAX_DIFF_AREA_SIZE)
