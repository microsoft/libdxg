/******************************Module*Header*******************************\
* Module Name: d3dkmt-wsl.cpp
*
* Client side stubs for the D3DKMT kernel call APIs.
*
* Copyright (c) Microsoft Corporation.
* Licensed under the MIT License.
*
\**************************************************************************/

#include <wsl/winadapter.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <uchar.h>
#include <unistd.h>
#include <cctype>
#include <cwchar>
#include <memory>
#include <algorithm>
#include <d3dkmthk.h>
#include <errno.h>

#define LINUX_PUBLIC __attribute__((visibility("default")))

#define RETURN_IF_NTSTATUS_FAILED(expr) do { NTSTATUS __statusRet = (expr); if (!NT_SUCCESS(__statusRet)) { return __statusRet; }} while (0)

int DxgFd = open("/dev/dxg", O_RDONLY | O_CLOEXEC);

NTSTATUS LinuxErrToNTSTATUS(int err)
{
    switch (err)
    {
    case EEXIST: return STATUS_OBJECT_NAME_COLLISION;
    case ENOMEM: return STATUS_NO_MEMORY;
    case EINVAL: return STATUS_INVALID_PARAMETER;
    case ENOENT: return STATUS_OBJECT_NAME_INVALID;
    case EAGAIN: return STATUS_TIMEOUT;
    case EOVERFLOW: return STATUS_BUFFER_TOO_SMALL;
    case ENODEV: return STATUS_DEVICE_REMOVED;
    case EACCES: return STATUS_ACCESS_DENIED;
    case ENOSYS: // fallthrough
    case EPERM: return STATUS_NOT_SUPPORTED;
    case EOPNOTSUPP: return STATUS_ILLEGAL_INSTRUCTION;
    case EBADF: return STATUS_INVALID_HANDLE;
    case EINPROGRESS: return STATUS_GRAPHICS_ALLOCATION_BUSY;
    case EPROTOTYPE: return STATUS_OBJECT_TYPE_MISMATCH;
    default: return STATUS_UNSUCCESSFUL;
    }
}

#define DEFINE_KMT(CamelCaseName, ARGSSTRUCT, IoctlId, Constness) \
    NTSTATUS APIENTRY LINUX_PUBLIC D3DKMT##CamelCaseName(ARGSSTRUCT Constness * pArgs) \
    { \
        if (DxgFd == -1) \
            return STATUS_UNSUCCESSFUL; \
        int ioctlret = ioctl(DxgFd, _IOWR('G', IoctlId, ARGSSTRUCT), pArgs); \
        if (ioctlret == -1 && (errno == ENOTTY || errno == EBADF)) { \
            close(DxgFd); \
            DxgFd = open("/dev/dxg", O_RDONLY | O_CLOEXEC); \
            ioctlret = ioctl(DxgFd, _IOWR('G', IoctlId, ARGSSTRUCT), pArgs); \
        } \
        if (ioctlret == -1) return LinuxErrToNTSTATUS(errno); \
        return ioctlret; \
    }
#define NONCONST

#define DEFINE_KMT_NOT_IMPL(CamelCaseName, ARGSSTRUCT, Constness) \
    NTSTATUS APIENTRY LINUX_PUBLIC D3DKMT##CamelCaseName(ARGSSTRUCT Constness * pArgs) \
    { \
        return STATUS_NOT_SUPPORTED; \
    }
#define DEFINE_KMT_NO_OP(CamelCaseName, ARGSSTRUCT, Constness) \
    NTSTATUS APIENTRY LINUX_PUBLIC D3DKMT##CamelCaseName(ARGSSTRUCT Constness * pArgs) \
    { \
        return STATUS_SUCCESS; \
    }

// Simple thunks
// The definitions of these ioctl opcodes come from /onecore/vm/wsl/lxcore/ioctl.h.
DEFINE_KMT(OpenAdapterFromLuid, D3DKMT_OPENADAPTERFROMLUID, 0x1, CONST)
DEFINE_KMT(CreateDevice, D3DKMT_CREATEDEVICE, 0x2, NONCONST)
DEFINE_KMT(CreateContext, D3DKMT_CREATECONTEXT, 0x3, NONCONST)
DEFINE_KMT(CreateContextVirtual, D3DKMT_CREATECONTEXTVIRTUAL, 0x4, NONCONST)
DEFINE_KMT(DestroyContext, D3DKMT_DESTROYCONTEXT, 0x5, CONST)
DEFINE_KMT(CreateAllocation2, D3DKMT_CREATEALLOCATION, 0x6, NONCONST)
DEFINE_KMT(CreatePagingQueue, D3DKMT_CREATEPAGINGQUEUE, 0x7, NONCONST)
DEFINE_KMT(ReserveGpuVirtualAddress, D3DDDI_RESERVEGPUVIRTUALADDRESS, 0x8, NONCONST)
// QueryAdapterInfo is 9, see below.
DEFINE_KMT(QueryVideoMemoryInfo, D3DKMT_QUERYVIDEOMEMORYINFO, 0xa, NONCONST)
DEFINE_KMT(MakeResident, D3DDDI_MAKERESIDENT, 0xb, NONCONST)
DEFINE_KMT(MapGpuVirtualAddress, D3DDDI_MAPGPUVIRTUALADDRESS, 0xc, NONCONST)
DEFINE_KMT(Escape, D3DKMT_ESCAPE, 0xd, CONST)
DEFINE_KMT(GetDeviceState, D3DKMT_GETDEVICESTATE, 0xe, NONCONST)
DEFINE_KMT(SubmitCommand, D3DKMT_SUBMITCOMMAND, 0xf, CONST)
DEFINE_KMT(CreateSynchronizationObject2, D3DKMT_CREATESYNCHRONIZATIONOBJECT2, 0x10, NONCONST)
DEFINE_KMT(SignalSynchronizationObject2, D3DKMT_SIGNALSYNCHRONIZATIONOBJECT2, 0x11, CONST)
DEFINE_KMT(WaitForSynchronizationObject2, D3DKMT_WAITFORSYNCHRONIZATIONOBJECT2, 0x12, CONST)
DEFINE_KMT(DestroyAllocation2, D3DKMT_DESTROYALLOCATION2, 0x13, CONST)
// Note: This is declared const in d3dkmthk.h even though it's modified by dxgkrnl
DEFINE_KMT(EnumAdapters2, D3DKMT_ENUMADAPTERS2, 0x14, CONST)
DEFINE_KMT(CloseAdapter, D3DKMT_CLOSEADAPTER, 0x15, CONST)
DEFINE_KMT(ChangeVideoMemoryReservation, D3DKMT_CHANGEVIDEOMEMORYRESERVATION, 0x16, CONST)
DEFINE_KMT(CreateHwContext, D3DKMT_CREATEHWCONTEXT, 0x17, NONCONST)
DEFINE_KMT(CreateHwQueue, D3DKMT_CREATEHWQUEUE, 0x18, NONCONST)
DEFINE_KMT(DestroyDevice, D3DKMT_DESTROYDEVICE, 0x19, CONST)
DEFINE_KMT(DestroyHwContext, D3DKMT_DESTROYHWCONTEXT, 0x1a, CONST)
DEFINE_KMT(DestroyHwQueue, D3DKMT_DESTROYHWQUEUE, 0x1b, CONST)
DEFINE_KMT(DestroyPagingQueue, D3DDDI_DESTROYPAGINGQUEUE, 0x1c, NONCONST)
DEFINE_KMT(DestroySynchronizationObject, D3DKMT_DESTROYSYNCHRONIZATIONOBJECT, 0x1d, CONST)
DEFINE_KMT(Evict, D3DKMT_EVICT, 0x1e, NONCONST)
DEFINE_KMT(FlushHeapTransitions, D3DKMT_FLUSHHEAPTRANSITIONS, 0x1f, NONCONST)
DEFINE_KMT(FreeGpuVirtualAddress, D3DKMT_FREEGPUVIRTUALADDRESS, 0x20, CONST)
DEFINE_KMT(GetContextInProcessSchedulingPriority, D3DKMT_GETCONTEXTINPROCESSSCHEDULINGPRIORITY, 0x21, NONCONST)
DEFINE_KMT(GetContextSchedulingPriority, D3DKMT_GETCONTEXTSCHEDULINGPRIORITY, 0x22, NONCONST)
DEFINE_KMT(GetSharedResourceAdapterLuid, D3DKMT_GETSHAREDRESOURCEADAPTERLUID, 0x23, NONCONST)
DEFINE_KMT(InvalidateCache, D3DKMT_INVALIDATECACHE, 0x24, CONST)
DEFINE_KMT(Lock2, D3DKMT_LOCK2, 0x25, NONCONST)
DEFINE_KMT(MarkDeviceAsError, D3DKMT_MARKDEVICEASERROR, 0x26, NONCONST)
DEFINE_KMT(OfferAllocations, D3DKMT_OFFERALLOCATIONS, 0x27, CONST)
DEFINE_KMT(OpenSynchronizationObject, D3DKMT_OPENSYNCHRONIZATIONOBJECT, 0x29, NONCONST)
DEFINE_KMT(QueryAllocationResidency, D3DKMT_QUERYALLOCATIONRESIDENCY, 0x2a, CONST)
DEFINE_KMT(QueryResourceInfo, D3DKMT_QUERYRESOURCEINFO, 0x2b, NONCONST)
DEFINE_KMT(ReclaimAllocations2, D3DKMT_RECLAIMALLOCATIONS2, 0x2c, NONCONST)
DEFINE_KMT(Render, D3DKMT_RENDER, 0x2d, NONCONST)
DEFINE_KMT(SetAllocationPriority, D3DKMT_SETALLOCATIONPRIORITY, 0x2e, CONST)
DEFINE_KMT(SetContextInProcessSchedulingPriority, D3DKMT_SETCONTEXTINPROCESSSCHEDULINGPRIORITY, 0x2f, CONST)
DEFINE_KMT(SetContextSchedulingPriority, D3DKMT_SETCONTEXTSCHEDULINGPRIORITY, 0x30, CONST)
DEFINE_KMT(SignalSynchronizationObjectFromCpu, D3DKMT_SIGNALSYNCHRONIZATIONOBJECTFROMCPU, 0x31, CONST)
DEFINE_KMT(SignalSynchronizationObjectFromGpu, D3DKMT_SIGNALSYNCHRONIZATIONOBJECTFROMGPU, 0x32, CONST)
DEFINE_KMT(SignalSynchronizationObjectFromGpu2, D3DKMT_SIGNALSYNCHRONIZATIONOBJECTFROMGPU2, 0x33, CONST)
DEFINE_KMT(SubmitCommandToHwQueue, D3DKMT_SUBMITCOMMANDTOHWQUEUE, 0x34, CONST)
DEFINE_KMT(SubmitSignalSyncObjectsToHwQueue, D3DKMT_SUBMITSIGNALSYNCOBJECTSTOHWQUEUE, 0x35, CONST)
DEFINE_KMT(SubmitWaitForSyncObjectsToHwQueue, D3DKMT_SUBMITWAITFORSYNCOBJECTSTOHWQUEUE, 0x36, CONST)
DEFINE_KMT(Unlock2, D3DKMT_UNLOCK2, 0x37, CONST)
DEFINE_KMT(UpdateAllocationProperty, D3DDDI_UPDATEALLOCPROPERTY, 0x38, NONCONST)
DEFINE_KMT(UpdateGpuVirtualAddress, D3DKMT_UPDATEGPUVIRTUALADDRESS, 0x39, CONST)
DEFINE_KMT(WaitForSynchronizationObjectFromCpu, D3DKMT_WAITFORSYNCHRONIZATIONOBJECTFROMCPU, 0x3a, CONST)
DEFINE_KMT(WaitForSynchronizationObjectFromGpu, D3DKMT_WAITFORSYNCHRONIZATIONOBJECTFROMGPU, 0x3b, CONST)
DEFINE_KMT(GetAllocationPriority, D3DKMT_GETALLOCATIONPRIORITY, 0x3c, CONST)
DEFINE_KMT(QueryClockCalibration, D3DKMT_QUERYCLOCKCALIBRATION, 0x3d, NONCONST)
DEFINE_KMT(EnumAdapters3, D3DKMT_ENUMADAPTERS3, 0x3e, NONCONST)
DEFINE_KMT(ShareObjectsInternal, D3DKMT_SHAREOBJECTS, 0x3f, NONCONST)
DEFINE_KMT(OpenSyncObjectFromNtHandle2, D3DKMT_OPENSYNCOBJECTFROMNTHANDLE2, 0x40, NONCONST)
DEFINE_KMT(QueryResourceInfoFromNtHandle, D3DKMT_QUERYRESOURCEINFOFROMNTHANDLE, 0x41, NONCONST)
DEFINE_KMT(OpenResourceFromNtHandle, D3DKMT_OPENRESOURCEFROMNTHANDLE, 0x42, NONCONST)
DEFINE_KMT(QueryStatistics, D3DKMT_QUERYSTATISTICS, 0x43, CONST)

// Not-implemented thunks (NT handles, or not in the compute interface subset we're using)
DEFINE_KMT_NOT_IMPL(GetMultisampleMethodList, D3DKMT_GETMULTISAMPLEMETHODLIST, NONCONST)
DEFINE_KMT_NOT_IMPL(SetQueuedLimit, D3DKMT_SETQUEUEDLIMIT, CONST)
DEFINE_KMT_NOT_IMPL(OpenNtHandleFromName, D3DKMT_OPENNTHANDLEFROMNAME, NONCONST)
DEFINE_KMT_NOT_IMPL(OpenSyncObjectNtHandleFromName, D3DKMT_OPENSYNCOBJECTNTHANDLEFROMNAME, NONCONST)
DEFINE_KMT_NOT_IMPL(OpenProtectedSessionFromNtHandle, D3DKMT_OPENPROTECTEDSESSIONFROMNTHANDLE, NONCONST)
DEFINE_KMT_NOT_IMPL(Lock, D3DKMT_LOCK, NONCONST)
DEFINE_KMT_NOT_IMPL(Unlock, D3DKMT_UNLOCK, CONST)
DEFINE_KMT_NOT_IMPL(SetDisplayPrivateDriverFormat, D3DKMT_SETDISPLAYPRIVATEDRIVERFORMAT, CONST)
DEFINE_KMT_NOT_IMPL(CreateProtectedSession, D3DKMT_CREATEPROTECTEDSESSION, NONCONST)
DEFINE_KMT_NOT_IMPL(DestroyProtectedSession, D3DKMT_DESTROYPROTECTEDSESSION, NONCONST)
DEFINE_KMT_NOT_IMPL(QueryProtectedSessionStatus, D3DKMT_QUERYPROTECTEDSESSIONSTATUS, NONCONST)
DEFINE_KMT_NOT_IMPL(CreateTrackedWorkload, D3DKMT_CREATETRACKEDWORKLOAD, NONCONST)
DEFINE_KMT_NOT_IMPL(DestroyTrackedWorkload, D3DKMT_DESTROYTRACKEDWORKLOAD, NONCONST)
DEFINE_KMT_NOT_IMPL(GetTrackedWorkloadStatistics, D3DKMT_GETTRACKEDWORKLOADSTATISTICS, NONCONST)
DEFINE_KMT_NOT_IMPL(ResetTrackedWorkloadStatistics, D3DKMT_RESETTRACKEDWORKLOADSTATISTICS, CONST)
DEFINE_KMT_NOT_IMPL(UpdateTrackedWorkload, D3DKMT_UPDATETRACKEDWORKLOAD, NONCONST)
DEFINE_KMT_NOT_IMPL(GetAvailableTrackedWorkloadIndex, D3DKMT_GETAVAILABLETRACKEDWORKLOADINDEX, NONCONST)
DEFINE_KMT_NOT_IMPL(CreateKeyedMutex2, D3DKMT_CREATEKEYEDMUTEX2, NONCONST)
DEFINE_KMT_NOT_IMPL(OpenKeyedMutex2, D3DKMT_OPENKEYEDMUTEX2, NONCONST)
DEFINE_KMT_NOT_IMPL(DestroyKeyedMutex, D3DKMT_DESTROYKEYEDMUTEX, CONST)
DEFINE_KMT_NOT_IMPL(AcquireKeyedMutex2, D3DKMT_ACQUIREKEYEDMUTEX2, NONCONST)
DEFINE_KMT_NOT_IMPL(ReleaseKeyedMutex2, D3DKMT_RELEASEKEYEDMUTEX2, NONCONST)
DEFINE_KMT_NOT_IMPL(QueryProtectedSessionInfoFromNtHandle, D3DKMT_QUERYPROTECTEDSESSIONINFOFROMNTHANDLE, NONCONST)
DEFINE_KMT_NOT_IMPL(Present, D3DKMT_PRESENT, NONCONST)
DEFINE_KMT_NOT_IMPL(SubmitPresentToHwQueue, D3DKMT_SUBMITPRESENTTOHWQUEUE, NONCONST)
DEFINE_KMT_NOT_IMPL(PinResources, D3DKMT_PINRESOURCES, NONCONST)
DEFINE_KMT_NOT_IMPL(UnpinResources, D3DKMT_UNPINRESOURCES, CONST)
DEFINE_KMT_NO_OP(SetStablePowerState, D3DKMT_SETSTABLEPOWERSTATE, CONST)

NTSTATUS LINUX_PUBLIC D3DKMTShareObjects(
    _In_range_(1, D3DKMT_MAX_OBJECTS_PER_HANDLE) UINT   cObjects,
    _In_reads_(cObjects) CONST D3DKMT_HANDLE *          hObjects,
    _In_ POBJECT_ATTRIBUTES                             pObjectAttributes,
    _In_ DWORD                                          dwDesiredAccess,
    _Out_ HANDLE *                                      phSharedNtHandle)
{
    D3DKMT_SHAREOBJECTS Args = {};
    Args.ObjectCount = cObjects;
    Args.ObjectHandleArray = hObjects;
    Args.pObjectAttributes = (VOID*)pObjectAttributes;
    Args.DesiredAccess = dwDesiredAccess;
    Args.pSharedNtHandle = phSharedNtHandle;
    return D3DKMTShareObjectsInternal(&Args);
}

// Remapping thunks
NTSTATUS LINUX_PUBLIC D3DKMTOpenResource(D3DKMT_OPENRESOURCE* pArg)
{
    // The input uses D3DDDI_OPENALLOCATIONINFO and kernel expects D3DDDI_OPENALLOCATIONINFO2.
    //
    const UINT NUM_ALLOC_INFO_ON_STACK = 4;
    D3DDDI_OPENALLOCATIONINFO2 LocalAllocInfoOnStack[NUM_ALLOC_INFO_ON_STACK] = {};
    D3DDDI_OPENALLOCATIONINFO2* pLocalAllocInfo = LocalAllocInfoOnStack;
    D3DDDI_OPENALLOCATIONINFO2* pOpenAllocationInfo2In = pArg->pOpenAllocationInfo2;

    if (pArg->NumAllocations > NUM_ALLOC_INFO_ON_STACK)
    {
        pLocalAllocInfo = new (std::nothrow) D3DDDI_OPENALLOCATIONINFO2[pArg->NumAllocations];
        if (pLocalAllocInfo == nullptr)
        {
            return STATUS_NO_MEMORY;
        }
    }
    for (UINT i = 0; i < pArg->NumAllocations; i++)
    {
        *(D3DDDI_OPENALLOCATIONINFO*)&pLocalAllocInfo[i] = pArg->pOpenAllocationInfo[i];
    }
    pArg->pOpenAllocationInfo2 = pLocalAllocInfo;

    int ioctlret = ioctl(DxgFd, _IOWR('G', 0x28, D3DKMT_OPENRESOURCE), pArg);
    NTSTATUS status = ioctlret == -1 ? LinuxErrToNTSTATUS(errno) : ioctlret;

    pArg->pOpenAllocationInfo2 = pOpenAllocationInfo2In;

    if (NT_SUCCESS(status))
    {
        for (UINT i = 0; i < pArg->NumAllocations; i++)
        {
            pArg->pOpenAllocationInfo[i] = *(D3DDDI_OPENALLOCATIONINFO*)&pLocalAllocInfo[i];
        }
    }

    if (pLocalAllocInfo != LocalAllocInfoOnStack)
    {
        delete [] pLocalAllocInfo;
    }
    return status;
}
NTSTATUS LINUX_PUBLIC D3DKMTDestroyAllocation(CONST D3DKMT_DESTROYALLOCATION* pArg)
{
    D3DKMT_DESTROYALLOCATION2 NewArg = {};
    if (!pArg)
    {
        return STATUS_INVALID_PARAMETER;
    }
    memcpy(&NewArg, pArg, sizeof(*pArg));
    return D3DKMTDestroyAllocation2(&NewArg);
}
NTSTATUS LINUX_PUBLIC D3DKMTCreateSynchronizationObject(D3DKMT_CREATESYNCHRONIZATIONOBJECT* pArg)
{
    D3DKMT_CREATESYNCHRONIZATIONOBJECT2 NewArg = {};
    if (!pArg)
    {
        return STATUS_INVALID_PARAMETER;
    }
    NewArg.hDevice = pArg->hDevice;
    NewArg.Info.Type = pArg->Info.Type;
    switch (NewArg.Info.Type)
    {
    case D3DDDI_SYNCHRONIZATION_MUTEX:

        NewArg.Info.SynchronizationMutex.InitialState = pArg->Info.SynchronizationMutex.InitialState;
        break;

    case D3DDDI_SEMAPHORE:

        NewArg.Info.Semaphore.MaxCount = pArg->Info.Semaphore.MaxCount;
        NewArg.Info.Semaphore.InitialCount = pArg->Info.Semaphore.InitialCount;
        break;

    default:
        return STATUS_INVALID_PARAMETER;
    }

    NTSTATUS status = D3DKMTCreateSynchronizationObject2(&NewArg);
    if (NT_SUCCESS(status))
    {
        pArg->hSyncObject = NewArg.hSyncObject;
    }
    return status;
}
NTSTATUS LINUX_PUBLIC D3DKMTWaitForSynchronizationObject(CONST D3DKMT_WAITFORSYNCHRONIZATIONOBJECT* pArg)
{
    D3DKMT_WAITFORSYNCHRONIZATIONOBJECT2 NewArg = {};
    if (!pArg)
    {
        return STATUS_INVALID_PARAMETER;
    }
    NewArg.hContext = pArg->hContext;
    NewArg.ObjectCount = pArg->ObjectCount;
    memcpy(&NewArg.ObjectHandleArray,
        pArg->ObjectHandleArray,
        std::min<UINT>(D3DDDI_MAX_OBJECT_WAITED_ON, NewArg.ObjectCount) * sizeof(D3DKMT_HANDLE));
    return D3DKMTWaitForSynchronizationObject2(&NewArg);
}
NTSTATUS LINUX_PUBLIC D3DKMTSignalSynchronizationObject(CONST D3DKMT_SIGNALSYNCHRONIZATIONOBJECT* pArg)
{
    D3DKMT_SIGNALSYNCHRONIZATIONOBJECT2 NewArg = {};
    if (!pArg)
    {
        return STATUS_INVALID_PARAMETER;
    }
    NewArg.hContext = pArg->hContext;
    NewArg.ObjectCount = pArg->ObjectCount;
    memcpy(&NewArg.ObjectHandleArray,
        pArg->ObjectHandleArray,
        std::min<UINT>(D3DDDI_MAX_OBJECT_SIGNALED, NewArg.ObjectCount) * sizeof(D3DKMT_HANDLE));
    NewArg.Flags = pArg->Flags;
    return D3DKMTSignalSynchronizationObject2(&NewArg);
}

void WidenStringAndHandleDriverStoreImpl(const char16_t *Input, wchar_t *Output, char* TempConversionString, size_t s)
{
    std::mbstate_t State{};
    size_t TempPos = 0;
    for (size_t i = 0; i < s; ++i)
    {
        if (!Input[i])
        {
            // For multi-string, just write a null
            TempConversionString[TempPos++] = 0;
        }
        else
        {
            TempPos += c16rtomb(&TempConversionString[TempPos], Input[i], &State);
        }
    }

    constexpr char DriverStoreString[] = "\\filerepository\\";
    size_t InputOffset = 0, OutputOffset = 0;
    while (InputOffset < s && OutputOffset < s && TempConversionString[0] != '\0')
    {
        size_t OutputChars;
        size_t InputChars = strlen(TempConversionString);
        if (auto InputPtr = std::search(TempConversionString, TempConversionString + InputChars,
            DriverStoreString, std::end(DriverStoreString) - 1,
                [](char a, char b) { return std::tolower(a) == b; });
            InputPtr != TempConversionString + InputChars)
        {
            constexpr wchar_t WslDriverStorePath[] = L"/usr/lib/wsl/drivers/";
            std::wcsncpy(Output, WslDriverStorePath, _countof(WslDriverStorePath));
            wchar_t* PostOutput = Output + _countof(WslDriverStorePath) - 1;
            size_t MaxNumOutputChars = s - (PostOutput - Output);
            OutputChars = std::mbstowcs(PostOutput, InputPtr + _countof(DriverStoreString) - 1, MaxNumOutputChars);
            std::replace(PostOutput, PostOutput + OutputChars, L'\\', L'/');

            OutputChars += _countof(WslDriverStorePath) - 1;
        }
        else
        {
            OutputChars = std::mbstowcs(Output, TempConversionString, s);
        }
        OutputOffset += OutputChars + 1;
        Output += OutputChars + 1;
        InputOffset += InputChars + 1;
        TempConversionString += InputChars + 1;
    }
}

template <size_t s> void WidenStringAndHandleDriverStore(const char16_t(&Input)[s], wchar_t(&Output)[s])
{
    char TempString[s] = {};
    WidenStringAndHandleDriverStoreImpl(Input, Output, TempString, s);
}

// QueryAdapterInfo with wide char handling
struct D3DKMT_UMDFILENAMEINFO_WCHAR16
{
    KMTUMDVERSION       Version;                // In: UMD version
    char16_t            UmdFileName[MAX_PATH];  // Out: UMD file name
};
void CopyAndConvertChars(D3DKMT_UMDFILENAMEINFO const& Input, D3DKMT_UMDFILENAMEINFO_WCHAR16& Output)
{
    Output.Version = Input.Version;
}
void CopyAndConvertChars(D3DKMT_UMDFILENAMEINFO_WCHAR16 const& Input, D3DKMT_UMDFILENAMEINFO& Output)
{
    WidenStringAndHandleDriverStore(Input.UmdFileName, Output.UmdFileName);
}

#pragma pack(push, 1)
struct DXGK_NODEMETADATA_WCHAR16
{
    DXGK_ENGINE_TYPE        EngineType;
    char16_t                FriendlyName[DXGK_MAX_METADATA_NAME_LENGTH];
    DXGK_NODEMETADATA_FLAGS Flags;
    BOOLEAN                 GpuMmuSupported;
    BOOLEAN                 IoMmuSupported;
};
struct D3DKMT_NODEMETADATA_WCHAR16
{
    UINT                        NodeOrdinalAndAdapterIndex; // In
    DXGK_NODEMETADATA_WCHAR16   NodeData;                   // Out
};
#pragma pack(pop)
void CopyAndConvertChars(D3DKMT_NODEMETADATA const& Input, D3DKMT_NODEMETADATA_WCHAR16& Output)
{
    Output.NodeOrdinalAndAdapterIndex = Input.NodeOrdinalAndAdapterIndex;
}
void CopyAndConvertChars(D3DKMT_NODEMETADATA_WCHAR16 const& Input, D3DKMT_NODEMETADATA& Output)
{
    std::copy(Input.NodeData.FriendlyName, std::end(Input.NodeData.FriendlyName), Output.NodeData.FriendlyName);
    Output.NodeData.EngineType = Input.NodeData.EngineType;
    Output.NodeData.Flags = Input.NodeData.Flags;
    Output.NodeData.GpuMmuSupported = Input.NodeData.GpuMmuSupported;
    Output.NodeData.IoMmuSupported = Input.NodeData.IoMmuSupported;
}

struct D3DKMT_OPENGLINFO_WCHAR16
{
    char16_t            UmdOpenGlIcdFileName[MAX_PATH];
    ULONG               Version;
    ULONG               Flags;
};
void CopyAndConvertChars(D3DKMT_OPENGLINFO const& Input, D3DKMT_OPENGLINFO_WCHAR16& Output)
{
    Output.Version = Input.Version;
    Output.Flags = Input.Flags;
}
void CopyAndConvertChars(D3DKMT_OPENGLINFO_WCHAR16 const& Input, D3DKMT_OPENGLINFO& Output)
{
    WidenStringAndHandleDriverStore(Input.UmdOpenGlIcdFileName, Output.UmdOpenGlIcdFileName);
}

struct D3DKMT_ADAPTERREGISTRYINFO_WCHAR16
{
    char16_t   AdapterString[MAX_PATH];
    char16_t   BiosString[MAX_PATH];
    char16_t   DacType[MAX_PATH];
    char16_t   ChipType[MAX_PATH];
};
void CopyAndConvertChars(D3DKMT_ADAPTERREGISTRYINFO const& Input, D3DKMT_ADAPTERREGISTRYINFO_WCHAR16& Output) {}
void CopyAndConvertChars(D3DKMT_ADAPTERREGISTRYINFO_WCHAR16 const& Input, D3DKMT_ADAPTERREGISTRYINFO& Output)
{
    WidenStringAndHandleDriverStore(Input.AdapterString, Output.AdapterString);
    WidenStringAndHandleDriverStore(Input.BiosString, Output.BiosString);
    WidenStringAndHandleDriverStore(Input.DacType, Output.DacType);
    WidenStringAndHandleDriverStore(Input.ChipType, Output.ChipType);
}

struct D3DKMT_DRIVER_DESCRIPTION_WCHAR16
{
    char16_t           DriverDescription[4096];     //out: The driver description
};
void CopyAndConvertChars(D3DKMT_DRIVER_DESCRIPTION const& Input, D3DKMT_DRIVER_DESCRIPTION_WCHAR16& Output) {}
void CopyAndConvertChars(D3DKMT_DRIVER_DESCRIPTION_WCHAR16 const& Input, D3DKMT_DRIVER_DESCRIPTION& Output)
{
    WidenStringAndHandleDriverStore(Input.DriverDescription, Output.DriverDescription);
}

struct D3DKMT_QUERY_ADAPTER_UNIQUE_GUID_WCHAR16
{
    char16_t           AdapterUniqueGUID[40];     //out: dxgkernel assigned GUID
};
void CopyAndConvertChars(D3DKMT_QUERY_ADAPTER_UNIQUE_GUID const& Input, D3DKMT_QUERY_ADAPTER_UNIQUE_GUID_WCHAR16& Output) {}
void CopyAndConvertChars(D3DKMT_QUERY_ADAPTER_UNIQUE_GUID_WCHAR16 const& Input, D3DKMT_QUERY_ADAPTER_UNIQUE_GUID& Output)
{
    std::copy(Input.AdapterUniqueGUID, std::end(Input.AdapterUniqueGUID), Output.AdapterUniqueGUID);
}

struct D3DDDI_QUERYREGISTRY_INFO_WCHAR16
{
    D3DDDI_QUERYREGISTRY_TYPE    QueryType;              // In
    D3DDDI_QUERYREGISTRY_FLAGS   QueryFlags;             // In
    char16_t                     ValueName[MAX_PATH];    // In
    ULONG                        ValueType;              // In
    ULONG                        PhysicalAdapterIndex;   // In
    ULONG                        OutputValueSize;        // Out. Number of bytes written to the output value or required in case of D3DDDI_QUERYREGISTRY_STATUS_BUFFER_OVERFLOW.
    D3DDDI_QUERYREGISTRY_STATUS  Status;                 // Out
    union {
        DWORD   OutputDword;                            // Out
        D3DKMT_ALIGN64 UINT64  OutputQword;                            // Out
        char16_t   OutputString[1];                     // Out. Dynamic array
        BYTE    OutputBinary[1];                        // Out. Dynamic array
    };
};

template <typename TInput, typename TOutput>
NTSTATUS CopyAndConvertCharsInput(const D3DKMT_QUERYADAPTERINFO* pArgs, std::unique_ptr<char[]>& spLocalMem, D3DKMT_QUERYADAPTERINFO& LocalArgs)
{
    if (pArgs->PrivateDriverDataSize != sizeof(TInput))
    {
        return STATUS_INVALID_PARAMETER;
    }
    spLocalMem.reset(new char[sizeof(TOutput)]{});
    LocalArgs.pPrivateDriverData = spLocalMem.get();
    LocalArgs.PrivateDriverDataSize = sizeof(TOutput);
    CopyAndConvertChars(*reinterpret_cast<const TInput*>(pArgs->pPrivateDriverData), *reinterpret_cast<TOutput*>(spLocalMem.get()));
    return STATUS_SUCCESS;
}

template <typename TInput, typename TOutput>
void CopyAndConvertCharsOutput(const D3DKMT_QUERYADAPTERINFO* pArgs, std::unique_ptr<char[]>& spLocalMem)
{
    CopyAndConvertChars(*reinterpret_cast<const TOutput*>(spLocalMem.get()), *reinterpret_cast<TInput*>(pArgs->pPrivateDriverData));
}

#define REG_SZ                      ( 1ul ) // Unicode nul terminated string
#define REG_EXPAND_SZ               ( 2ul ) // Unicode nul terminated string
                                            // (with environment variable references)
#define REG_MULTI_SZ                ( 7ul ) // Multiple Unicode strings

template <>
NTSTATUS CopyAndConvertCharsInput<D3DDDI_QUERYREGISTRY_INFO, D3DDDI_QUERYREGISTRY_INFO_WCHAR16>(const D3DKMT_QUERYADAPTERINFO* pArgs, std::unique_ptr<char[]>& spLocalMem, D3DKMT_QUERYADAPTERINFO& LocalArgs)
{
    if (pArgs->PrivateDriverDataSize < sizeof(D3DDDI_QUERYREGISTRY_INFO))
    {
        return STATUS_INVALID_PARAMETER;
    }
    auto pInput = reinterpret_cast<D3DDDI_QUERYREGISTRY_INFO*>(pArgs->pPrivateDriverData);
    bool ExtraBytesAreChars =
        pInput->QueryType == D3DDDI_QUERYREGISTRY_DRIVERSTOREPATH ||
        pInput->QueryType == D3DDDI_QUERYREGISTRY_DRIVERIMAGEPATH ||
        pInput->ValueType == REG_SZ || pInput->ValueType == REG_EXPAND_SZ || pInput->ValueType == REG_MULTI_SZ;

    SIZE_T ExtraBytesIn = pArgs->PrivateDriverDataSize - sizeof(D3DDDI_QUERYREGISTRY_INFO);
    SIZE_T ExtraBytesOut = ExtraBytesAreChars ?
        (ExtraBytesIn / sizeof(wchar_t) * sizeof(char16_t)) : ExtraBytesIn;

    SIZE_T OutputSize = sizeof(D3DDDI_QUERYREGISTRY_INFO_WCHAR16) + ExtraBytesOut;
    spLocalMem.reset(new char[OutputSize]);
    auto pOutput = reinterpret_cast<D3DDDI_QUERYREGISTRY_INFO_WCHAR16*>(spLocalMem.get());

    LocalArgs.pPrivateDriverData = spLocalMem.get();
    LocalArgs.PrivateDriverDataSize = OutputSize;

    pOutput->QueryType = pInput->QueryType;
    pOutput->QueryFlags = pInput->QueryFlags;
    pOutput->ValueType = pInput->ValueType;
    pOutput->PhysicalAdapterIndex = pInput->PhysicalAdapterIndex;
    pOutput->OutputValueSize = 0;
    pOutput->Status = D3DDDI_QUERYREGISTRY_STATUS_FAIL;
    if (pInput->QueryType == D3DDDI_QUERYREGISTRY_SERVICEKEY ||
        pInput->QueryType == D3DDDI_QUERYREGISTRY_ADAPTERKEY)
    {
        for (UINT i = 0; i < _countof(pOutput->ValueName); ++i)
        {
            pOutput->ValueName[i] = (char16_t)pInput->ValueName[i];
            if (pOutput->ValueName[i] != pInput->ValueName[i])
            {
                // Not supporting anything that doesn't fit in UCS-2.
                return STATUS_INVALID_PARAMETER;
            }
        }
    }
    return STATUS_SUCCESS;
}
template <>
void CopyAndConvertCharsOutput<D3DDDI_QUERYREGISTRY_INFO, D3DDDI_QUERYREGISTRY_INFO_WCHAR16>(const D3DKMT_QUERYADAPTERINFO* pArgs, std::unique_ptr<char[]>& spLocalMem)
{
    auto pInput = reinterpret_cast<D3DDDI_QUERYREGISTRY_INFO*>(pArgs->pPrivateDriverData);
    auto pOutput = reinterpret_cast<D3DDDI_QUERYREGISTRY_INFO_WCHAR16*>(spLocalMem.get());
    bool ExtraBytesAreChars =
        pInput->QueryType == D3DDDI_QUERYREGISTRY_DRIVERSTOREPATH ||
        pInput->QueryType == D3DDDI_QUERYREGISTRY_DRIVERIMAGEPATH ||
        pInput->ValueType == REG_SZ || pInput->ValueType == REG_EXPAND_SZ || pInput->ValueType == REG_MULTI_SZ;

    pInput->Status = pOutput->Status;
    pInput->OutputValueSize = ExtraBytesAreChars ?
        (pOutput->OutputValueSize / sizeof(char16_t) * sizeof(wchar_t)) : pOutput->OutputValueSize;
    if (pOutput->Status == D3DDDI_QUERYREGISTRY_STATUS_SUCCESS)
    {
        if (ExtraBytesAreChars)
        {
            char TempString[MAX_PATH] = {};
            std::unique_ptr<char> LargeTempString;
            size_t OutputChars = pOutput->OutputValueSize / sizeof(char16_t);
            if (OutputChars > MAX_PATH)
            {
                LargeTempString.reset(new char[OutputChars]);
            }
            WidenStringAndHandleDriverStoreImpl(pOutput->OutputString, pInput->OutputString,
                LargeTempString ? LargeTempString.get() : TempString, OutputChars);
        }
        else
        {
            memcpy(&pInput->OutputDword, &pOutput->OutputDword, pOutput->OutputValueSize);
        }
    }
}

#define QUERYADAPTERINFO_STRINGCONVERTCASES(MACRO) \
    MACRO(KMTQAITYPE_UMDRIVERNAME, D3DKMT_UMDFILENAMEINFO) \
    MACRO(KMTQAITYPE_UMOPENGLINFO, D3DKMT_OPENGLINFO) \
    MACRO(KMTQAITYPE_ADAPTERREGISTRYINFO, D3DKMT_ADAPTERREGISTRYINFO) \
    MACRO(KMTQAITYPE_ADAPTERREGISTRYINFO_RENDER, D3DKMT_ADAPTERREGISTRYINFO) \
    MACRO(KMTQAITYPE_DRIVER_DESCRIPTION, D3DKMT_DRIVER_DESCRIPTION) \
    MACRO(KMTQAITYPE_DRIVER_DESCRIPTION_RENDER, D3DKMT_DRIVER_DESCRIPTION) \
    MACRO(KMTQAITYPE_QUERY_ADAPTER_UNIQUE_GUID, D3DKMT_QUERY_ADAPTER_UNIQUE_GUID) \
    MACRO(KMTQAITYPE_NODEMETADATA, D3DKMT_NODEMETADATA) \
    MACRO(KMTQAITYPE_QUERYREGISTRY, D3DDDI_QUERYREGISTRY_INFO)

NTSTATUS APIENTRY LINUX_PUBLIC D3DKMTQueryAdapterInfo(CONST D3DKMT_QUERYADAPTERINFO* pArgs)
{
    if (DxgFd == -1) return STATUS_UNSUCCESSFUL;

    D3DKMT_QUERYADAPTERINFO LocalArgs = *pArgs;
    std::unique_ptr<char[]> spLocalMem;

    try
    {
        if constexpr (sizeof(wchar_t) != 2)
        {
            switch (pArgs->Type)
            {
#define QUERYADAPTERINFO_STRINGCONVERTINPUT(QAITYPE, KMTSTRUCT) \
    case QAITYPE: RETURN_IF_NTSTATUS_FAILED((CopyAndConvertCharsInput<KMTSTRUCT, KMTSTRUCT##_WCHAR16>(pArgs, spLocalMem, LocalArgs))); break;
                QUERYADAPTERINFO_STRINGCONVERTCASES(QUERYADAPTERINFO_STRINGCONVERTINPUT)
#undef QUERYADAPTERINFO_STRINGCONVERTINPUT
    default:
        break;
            }
        }
    }
    catch (std::bad_alloc&) { return STATUS_NO_MEMORY; }

    int ioctlret = ioctl(DxgFd, _IOWR('G', 0x09, D3DKMT_QUERYADAPTERINFO), &LocalArgs);
    NTSTATUS status = ioctlret == -1 ? LinuxErrToNTSTATUS(errno) : ioctlret;

    if constexpr (sizeof(wchar_t) != 2)
    {
        if (NT_SUCCESS(status))
        {
            switch (pArgs->Type)
            {
#define QUERYADAPTERINFO_STRINGCONVERTOUTPUT(QAITYPE, KMTSTRUCT) \
    case QAITYPE: CopyAndConvertCharsOutput<KMTSTRUCT, KMTSTRUCT##_WCHAR16>(pArgs, spLocalMem); break;
                QUERYADAPTERINFO_STRINGCONVERTCASES(QUERYADAPTERINFO_STRINGCONVERTOUTPUT)
#undef QUERYADAPTERINFO_STRINGCONVERTOUTPUT
    default:
        break;
            }
        }
    }

    return status;
}

static BOOL HexStringToDword(const wchar_t*& lpsz, DWORD& Value,
    int cDigits, WCHAR chDelim)
{
    int Count;

    Value = 0;
    for (Count = 0; Count < cDigits; Count++, lpsz++)
    {
        if (*lpsz >= '0' && *lpsz <= '9')
            Value = (Value << 4) + *lpsz - '0';
        else if (*lpsz >= 'A' && *lpsz <= 'F')
            Value = (Value << 4) + *lpsz - 'A' + 10;
        else if (*lpsz >= 'a' && *lpsz <= 'f')
            Value = (Value << 4) + *lpsz - 'a' + 10;
        else
            return(FALSE);
    }

    if (chDelim != 0)
        return *lpsz++ == chDelim;
    else
        return TRUE;
}
static BOOL wUUIDFromString(const wchar_t* lpsz, GUID* pguid)
{
    DWORD dw;

    if (!HexStringToDword(lpsz, pguid->Data1, sizeof(DWORD) * 2, '-'))
        return FALSE;

    if (!HexStringToDword(lpsz, dw, sizeof(WORD) * 2, '-'))
        return FALSE;

    pguid->Data2 = (WORD)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(WORD) * 2, '-'))
        return FALSE;

    pguid->Data3 = (WORD)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE) * 2, 0))
        return FALSE;

    pguid->Data4[0] = (BYTE)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE) * 2, '-'))
        return FALSE;

    pguid->Data4[1] = (BYTE)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE) * 2, 0))
        return FALSE;

    pguid->Data4[2] = (BYTE)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE) * 2, 0))
        return FALSE;

    pguid->Data4[3] = (BYTE)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE) * 2, 0))
        return FALSE;

    pguid->Data4[4] = (BYTE)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE) * 2, 0))
        return FALSE;

    pguid->Data4[5] = (BYTE)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE) * 2, 0))
        return FALSE;

    pguid->Data4[6] = (BYTE)dw;
    if (!HexStringToDword(lpsz, dw, sizeof(BYTE) * 2, 0))
        return FALSE;

    pguid->Data4[7] = (BYTE)dw;

    return TRUE;
}

HRESULT IIDFromString(const wchar_t* lpsz, IID* lpiid)
{
    if (*lpsz++ != '{')
        return E_INVALIDARG;

    if (wUUIDFromString(lpsz, lpiid) != TRUE)
        return E_INVALIDARG;

    lpsz += 36;

    if (*lpsz++ != '}')
        return E_INVALIDARG;

    if (*lpsz != '\0')
    {
        return E_INVALIDARG;
    }
    return S_OK;
}
