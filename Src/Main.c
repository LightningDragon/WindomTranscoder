#define WIN32_LEAN_AND_MEAN

#define CHUNK_SIZE 0x100000
#define WINDOM_CIPHER_KEY 0x0B7E7759
#define SEEDMOD_CIPHER_KEY 0x95127634
#define MSVMOD_CIPHER_KEY 0x19870430
#define SEEDMOD205_CIPHER_KEY 0xAC510B91
#define UNITEDMOD_CIPHER_KEY 0x13322366
#define RAIDMOD_CIPHER_KEY 0xEF452301
#define THEEPICOFWAR_CIPHER_KEY 0x33333323
#define UN_EVO_CIPHER_KEY 0x33322166

#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>

#include "resource.h"

OPENFILENAME FileDialog = { 0 };
BROWSEINFO FolderDialog = { 0 };

char Path[MAX_PATH];
char Buffer[CHUNK_SIZE / sizeof(int)];
int Keys[] = { WINDOM_CIPHER_KEY, SEEDMOD_CIPHER_KEY, MSVMOD_CIPHER_KEY, SEEDMOD205_CIPHER_KEY, UNITEDMOD_CIPHER_KEY, RAIDMOD_CIPHER_KEY, THEEPICOFWAR_CIPHER_KEY, UN_EVO_CIPHER_KEY};
char* ValidExtensions[] = { ".ani", ".fx", ".mpd", ".sdt", ".hod", ".ANI", ".FX", ".MPD", ".SDT", ".HOD" };

// XORs a file an int at a time till no longer possible
void CipherFile(int key, char* path)
{
	FILE* fileHandle = fopen(path, "r+b");

	if (fileHandle != NULL)
	{
		int isAtEndOfFile = TRUE;
		unsigned int chunkSize;
		unsigned int index;

		while (isAtEndOfFile)
		{
			chunkSize = fread(Buffer, sizeof(char), sizeof(Buffer), fileHandle);

			if (chunkSize < 1)
            {
                break;
            }

			if (feof(fileHandle))
			{
				isAtEndOfFile = FALSE;

				for (index = 0; index < (chunkSize - 4); index += 4)
				{
					*(int*)(Buffer + index) ^= key;
				}
			}
			else
			{
				for (index = 0; index < chunkSize; index += 4)
				{
					*(int*)(Buffer + index) ^= key;
				}
			}

			fseek(fileHandle, -chunkSize, SEEK_CUR);
			fwrite(Buffer, sizeof(char), chunkSize, fileHandle);
		}

		fclose(fileHandle);
	}
}

// Checks if a string ends with a valid file extension
int HasValidExtension(char* path)
{
	int length = strlen(path);

	if (length > 4)
	{
		char* extention = (path + length - 4);
		unsigned int index;

		for (index = 0; index < 10; index++)
		{
			if (strcmp(extention, ValidExtensions[index]) == FALSE)
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

// Callback that handles the messages for the main dialog of the application
int __stdcall DialogProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (Message)
	{
	case WM_COMMAND:
		switch (wParam)
		{
		case IDCANCEL:
			PostQuitMessage(0);
			break;
		case IDC_FIND_FILE:
			if (GetOpenFileName(&FileDialog))
			{
				SetDlgItemText(hwnd, IDC_TEXT, FileDialog.lpstrFile);
			}
			break;
		case IDC_FIND_FOLDER:
			if (SHGetPathFromIDList(SHBrowseForFolder(&FolderDialog), FileDialog.lpstrFile))
			{
				SetDlgItemText(hwnd, IDC_TEXT, FolderDialog.pszDisplayName);
			}
			break;
		case IDC_ENCODE_DECODE:
			if (GetWindowTextLength(GetDlgItem(hwnd, IDC_TEXT)) > 0)
			{
				struct stat state;
				GetDlgItemText(hwnd, IDC_TEXT, Path, sizeof(Path));
				stat(Path, &state);

				if (S_ISREG(state.st_mode))
				{
					CipherFile(Keys[SendDlgItemMessage(hwnd, IDC_DROP_DOWN, CB_GETCURSEL, 0, 0)], Path);
					MessageBox(hwnd, "Transcode Completed", "Done", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
				}
				else if (S_ISDIR(state.st_mode))
				{
					DIR* dir = opendir(Path);
					char str[MAX_PATH];
					int key = Keys[SendDlgItemMessage(hwnd, IDC_DROP_DOWN, CB_GETCURSEL, 0, 0)];
					struct dirent* entry;

					while ((entry = readdir(dir)))
					{
						sprintf(str, "%s%s%s", Path, "\\", entry->d_name);

						if (IsDlgButtonChecked(hwnd, IDC_CHECK_BOX) == BST_CHECKED)
						{
							if (HasValidExtension(str) == TRUE)
							{
								CipherFile(key, str);
							}
						}
						else
						{
							CipherFile(key, str);
						}
					}

					closedir(dir);
					MessageBox(hwnd, "Transcode Completed", "Done", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
				}
				else
				{
					MessageBox(hwnd, "Invalid File or Directory", "Error", MB_OK | MB_ICONWARNING | MB_APPLMODAL);
				}
			}
			break;
		default:
			break;
		}

		Path[0] = 0;
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

// Initializes the main window and dialogs
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	HICON iconhadle = LoadIcon(hInstance, IDI_APPLICATION);
	HWND dialogHadle = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc);

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	FileDialog.lStructSize = sizeof(OPENFILENAME);
	FileDialog.hwndOwner = dialogHadle;
	FileDialog.lpstrFile = Path;
	FileDialog.nMaxFile = sizeof(Path);
	FileDialog.Flags = OFN_NOCHANGEDIR;

	FolderDialog.hwndOwner = dialogHadle;
	FolderDialog.pszDisplayName = Path;
	FolderDialog.ulFlags = BIF_USENEWUI;

	SendMessage(dialogHadle, WM_SETICON, ICON_SMALL, (LPARAM)iconhadle);
	SendMessage(dialogHadle, WM_SETICON, ICON_BIG, (LPARAM)iconhadle);
	SendDlgItemMessage(dialogHadle, IDC_DROP_DOWN, CB_ADDSTRING, 0, (LPARAM)"Windom Mode");
	SendDlgItemMessage(dialogHadle, IDC_DROP_DOWN, CB_ADDSTRING, 0, (LPARAM)"SeedMod Mode");
	SendDlgItemMessage(dialogHadle, IDC_DROP_DOWN, CB_ADDSTRING, 0, (LPARAM)"MSV_MOD Mode");
	SendDlgItemMessage(dialogHadle, IDC_DROP_DOWN, CB_ADDSTRING, 0, (LPARAM)"SeedMod 2.0.5");
	SendDlgItemMessage(dialogHadle, IDC_DROP_DOWN, CB_ADDSTRING, 0, (LPARAM)"United Mod");
	SendDlgItemMessage(dialogHadle, IDC_DROP_DOWN, CB_ADDSTRING, 0, (LPARAM)"Raid Mod");
	SendDlgItemMessage(dialogHadle, IDC_DROP_DOWN, CB_ADDSTRING, 0, (LPARAM)"The Epic of War");
	SendDlgItemMessage(dialogHadle, IDC_DROP_DOWN, CB_ADDSTRING, 0, (LPARAM)"UN Evo");
	SendDlgItemMessage(dialogHadle, IDC_DROP_DOWN, CB_SETCURSEL, 0, 0);
	CheckDlgButton(dialogHadle, IDC_CHECK_BOX, BST_CHECKED);

	while (GetMessage(&msg, NULL, 0, 0) == TRUE)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}
