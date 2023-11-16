#include <iostream>
#include <windows.h>

int main() 
{
    // Открытие диалога выбора файла
    OPENFILENAME ofn;
    char szFile[260];

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Disk Image Files\0*.img\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE) 
    {
        // Создание виртуального привода
        char virtualDriveLetter = 'Z';  // Выбираем букву привода
        std::string virtualDrivePath = "\\\\.\\";
        virtualDrivePath += virtualDriveLetter;
        virtualDrivePath += ':';

        HANDLE hDevice = CreateFileA(
            virtualDrivePath.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_OVERLAPPED,
            NULL
        );

        if (hDevice != INVALID_HANDLE_VALUE) {
            // Монтирование образа диска
            if (DeviceIoControl(hDevice, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, NULL, NULL) != 0) {
                std::string imagePath = ofn.lpstrFile;
                imagePath = "\\\\.\\" + imagePath;

                if (DeviceIoControl(hDevice, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, NULL, NULL) != 0) {
                    if (DeviceIoControl(hDevice, FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0, NULL, NULL) != 0) {
                        if (DeviceIoControl(hDevice, IOCTL_DISK_DELETE_DRIVE_LAYOUT, NULL, 0, NULL, 0, NULL, NULL) != 0) {
                            if (DeviceIoControl(hDevice, IOCTL_DISK_CREATE_DISK, NULL, 0, NULL, 0, NULL, NULL) != 0) {
                                if (DeviceIoControl(hDevice, IOCTL_DISK_UPDATE_PROPERTIES, NULL, 0, NULL, 0, NULL, NULL) != 0) {
                                    if (DeviceIoControl(hDevice, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, NULL, NULL) != 0) {
                                        if (DeviceIoControl(hDevice, FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0, NULL, NULL) != 0) {
                                            HANDLE hDisk = CreateFileA(
                                                imagePath.c_str(),
                                                GENERIC_READ,
                                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                NULL,
                                                OPEN_EXISTING,
                                                FILE_FLAG_RANDOM_ACCESS,
                                                NULL
                                            );

                                            if (hDisk != INVALID_HANDLE_VALUE) {
                                                DWORD bytesRead;
                                                DWORD bytesWritten;

                                                char buffer[4096];

                                                if (ReadFile(hDisk, buffer, sizeof(buffer), &bytesRead, NULL)) {
                                                    if (WriteFile(hDevice, buffer, bytesRead, &bytesWritten, NULL)) {
                                                        std::cout << "Disk image mounted successfully." << std::endl;
                                                    } else {
                                                        std::cerr << "Error writing to virtual drive: " << GetLastError() << std::endl;
                                                    }
                                                } else {
                                                    std::cerr << "Error reading from disk image: " << GetLastError() << std::endl;
                                                }

                                                CloseHandle(hDisk);
                                            } else {
                                                std::cerr << "Error opening disk image: " << GetLastError() << std::endl;
                                            }
                                        } else {
                                            std::cerr << "Error unlocking volume: " << GetLastError() << std::endl;
                                        }
                                    } else {
                                        std::cerr << "Error dismounting volume: " << GetLastError() << std::endl;
                                    }
                                } else {
                                    std::cerr << "Error updating disk properties: " << GetLastError() << std::endl;
                                }
                            } else {
                                std::cerr << "Error creating disk: " << GetLastError() << std::endl;
                            }
                        } else {
                            std::cerr << "Error deleting drive layout: " << GetLastError() << std::endl;
                        }
                    } else {
                        std::cerr << "Error unlocking volume: " << GetLastError() << std::endl;
                    }
                } else {
                    std::cerr << "Error dismounting volume: " << GetLastError() << std::endl;
                }
            } else {
                std::cerr << "Error locking volume: " << GetLastError() << std::endl;
            }

            CloseHandle(hDevice);
        } else {
            std::cerr << "Error opening virtual drive: " << GetLastError() << std::endl;
        }
    } else {
        std::cerr << "Error selecting disk image file." << std::endl;
    }

    return 0;
}
