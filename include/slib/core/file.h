/*
 *   Copyright (c) 2008-2018 SLIBIO <https://github.com/SLIBIO>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *   THE SOFTWARE.
 */

#ifndef CHECKHEADER_SLIB_CORE_FILE
#define CHECKHEADER_SLIB_CORE_FILE

#include "definition.h"

#include "list.h"
#include "io.h"

typedef sl_reg sl_file;
#define SLIB_FILE_INVALID_HANDLE (sl_file)(-1)

namespace slib
{
	
	class FileMode
	{
	public:
		int value;
		SLIB_MEMBERS_OF_FLAGS(FileMode, value)
		
		enum {
			Read = 1,
			Write = 2,

			NotCreate = 0x00001000,
			NotTruncate = 0x00002000,
			SeekToEnd = 0x10000000,
			HintRandomAccess = 0x20000000,

			ReadWrite = Read | Write,
			Append = Write | NotTruncate | SeekToEnd,
			RandomAccess = Read | Write | NotTruncate | HintRandomAccess,
			RandomRead = Read | HintRandomAccess
		};
	};

	class FileAttributes
	{
	public:
		int value;
		SLIB_MEMBERS_OF_FLAGS(FileAttributes, value)

		enum {
			Default = 0,
			Directory = 1,
			Hidden = 2,
			NotExist = 0x8000
		};
	};
	
	class FilePermissions
	{
		int value;
		SLIB_MEMBERS_OF_FLAGS(FilePermissions, value)
		
		enum {
			ReadByOthers = 0x0001,
			WriteByOthers = 0x0002,
			ExecuteByOthers = 0x0004,
			Others = ReadByOthers | WriteByOthers | ExecuteByOthers,

			ReadByGroup = 0x0008,
			WriteByGroup = 0x0010,
			ExecuteByGroup = 0x0020,
			Group = ReadByGroup | WriteByGroup | ExecuteByGroup,

			ReadByUser = 0x0040,
			WriteByUser = 0x0080,
			ExecuteByUser = 0x0100,
			User = ReadByUser | WriteByUser | ExecuteByUser,

			Read = ReadByUser | ReadByGroup | ReadByOthers,
			Write = WriteByUser | WriteByGroup | WriteByOthers,
			Execute = ExecuteByUser | ExecuteByGroup | ExecuteByOthers,

			None = 0,
			All = Read | Write | Execute,
	
			ShareRead = 0x1000,
			ShareWrite = 0x2000,
			ShareReadWrite = ShareRead | ShareWrite,
			ShareDelete = 0x4000,
			ShareAll = ShareRead | ShareWrite | ShareDelete

		};
	
	};
	
	class SLIB_EXPORT File : public IO
	{
		SLIB_DECLARE_OBJECT
	
	private:
		String m_path;
		sl_file m_file;
	
	private:
		File();
	
		~File();
	
	public:
		static Ref<File> open(const String& filePath, const FileMode& mode, const FilePermissions& permissions);

		static Ref<File> open(const String& filePath, const FileMode& mode);
	
		static Ref<File> openForRead(const String& filePath, sl_bool flagShareRead = sl_true);

		static Ref<File> openForWrite(const String& filePath);

		static Ref<File> openForReadWrite(const String& filePath);

		static Ref<File> openForAppend(const String& filePath);

		static Ref<File> openForRandomAccess(const String& filePath);
	
		static Ref<File> openForRandomRead(const String& filePath, sl_bool flagShareRead = sl_true);

		/*
			Physical Disks and Volumes
		 
				"\\\\.\\PhysicalDrive0"  (Win32)
				"\\\\.\\CdRom0"
				"\\\\.\\A:"
		 
		 		"/dev/disk0"  (macOS)
		 		"/dev/sda1"   (Linux)
		*/
		static Ref<File> openDevice(const String16& path, sl_bool flagRead, sl_bool flagWrite);

	public:
		void close() override;

		sl_bool isOpened() const;
	
		String getPath() const;
	
		sl_file getHandle() const;

		sl_uint64 getPosition() override;

		sl_uint64 getSize() override;
		
		sl_bool seek(sl_int64 offset, SeekPosition from) override;
	

		sl_int32 read32(void* buf, sl_uint32 size) override;

		sl_int32 write32(const void* buf, sl_uint32 size) override;
	
	
		// works only if the file is already opened
		sl_bool setSize(sl_uint64 size) override;

		
		static sl_uint64 getSize(sl_file fd);
		
		static sl_uint64 getSize(const String& path);
		
		static sl_uint64 getDiskSize(sl_file fd);
		
		static sl_uint64 getDiskSize(const String& path);
		
		
		sl_bool lock();

		sl_bool unlock();
	
		sl_uint64 getDiskSize();

		
		Time getModifiedTime();

		Time getAccessedTime();

		Time getCreatedTime();

		static Time getModifiedTime(const String& filePath);

		static Time getAccessedTime(const String& filePath);

		static Time getCreatedTime(const String& filePath);

		static sl_bool setModifiedTime(const String& filePath, Time time);

		static sl_bool setAccessedTime(const String& filePath, Time time);

		static sl_bool setCreatedTime(const String& filePath, Time time);
	

		static FileAttributes getAttributes(const String& filePath);

		static sl_bool exists(const String& filePath);
	
		static sl_bool isFile(const String& filePath);
	
		static sl_bool isDirectory(const String& filePath);

		static sl_bool isHidden(const String& filePath);
	
		static sl_bool setHidden(const String& filePath, sl_bool flagHidden = sl_true);
	

		static sl_bool createDirectory(const String& dirPath, sl_bool flagErrorOnCreateExistingDirectory = sl_false);

		static sl_bool createDirectories(const String& dirPath);

		static sl_bool deleteFile(const String& filePath, sl_bool flagErrorOnDeleteNotExistingFile = sl_false);
	

		// Deletes the directory and its sub-directories and files
		static sl_bool deleteDirectoryRecursively(const String& dirPath);

		// Changes the path of file or directory. Don't replace the existing file.
		static sl_bool rename(const String& filePathOriginal, const String& filePathNew);
	

		static List<String> getFiles(const String& dirPath);
	
		static List<String> getAllDescendantFiles(const String& dirPath);
	

		Memory readAllBytes(sl_size maxSize = SLIB_SIZE_MAX);
		
		static Memory readAllBytes(const String& path, sl_size maxSize = SLIB_SIZE_MAX);
		
		String readAllTextUTF8(sl_size maxSize = SLIB_SIZE_MAX);

		static String readAllTextUTF8(const String& path, sl_size maxSize = SLIB_SIZE_MAX);

		String16 readAllTextUTF16(sl_bool flagBigEndian = sl_false, sl_size maxSize = SLIB_SIZE_MAX);
		
		static String16 readAllTextUTF16(const String& path, sl_bool flagBigEndian = sl_false, sl_size maxSize = SLIB_SIZE_MAX);
		
		String readAllText(Charset* outCharset = sl_null, sl_size maxSize = SLIB_SIZE_MAX);

		static String readAllText(const String& path, Charset* outCharset = sl_null, sl_size maxSize = SLIB_SIZE_MAX);
		
		String16 readAllText16(Charset* outCharset = sl_null, sl_size maxSize = SLIB_SIZE_MAX);

		static String16 readAllText16(const String& path, Charset* outCharset = sl_null, sl_size maxSize = SLIB_SIZE_MAX);
	
		static sl_size writeAllBytes(const String& path, const void* buf, sl_size size);

		static sl_size writeAllBytes(const String& path, const Memory& mem);

		static sl_bool writeAllTextUTF8(const String& path, const String& text, sl_bool flagWriteByteOrderMark = sl_false);

		static sl_bool writeAllTextUTF16LE(const String& path, const String16& text, sl_bool flagWriteByteOrderMark = sl_false);

		static sl_bool writeAllTextUTF16BE(const String& path, const String16& text, sl_bool flagWriteByteOrderMark = sl_false);

		static sl_size appendAllBytes(const String& path, const void* buf, sl_size size);

		static sl_size appendAllBytes(const String& path, const Memory& mem);

		static sl_bool appendAllTextUTF8(const String& path, const String& text);

		static sl_bool appendAllTextUTF16LE(const String& path, const String16& text);

		static sl_bool appendAllTextUTF16BE(const String& path, const String16& text);

	
		static String getParentDirectoryPath(const String& path);

		static String getFileName(const String& path);

		static String getFileExtension(const String& path);

		static String getFileNameOnly(const String& path);

		static String normalizeDirectoryPath(const String& path);
	

		// converts any invalid characters (0~0x1f, 0x7f~0x9f, :*?"<>|\/) into "_"
		static String makeSafeFileName(const String& fileName);
	
		// converts any invalid characters (0~0x1f, 0x7f~0x9f, :*?"<>|) into "_"
		static String makeSafeFilePath(const String& filePath);
	
		static String findParentPathContainingFile(const String& basePath, const String& filePath, sl_uint32 nDeep = SLIB_UINT32_MAX);
	
#if defined(SLIB_PLATFORM_IS_UNIX)
		static sl_bool setNonBlocking(int fd, sl_bool flag);
#endif
		
		static String getRealPath(const String& filePath);

	private:
		static sl_file _open(const String& filePath, const FileMode& mode, const FilePermissions& permissions);

		static sl_bool _close(sl_file file);

		static sl_bool _createDirectory(const String& dirPath);

		static sl_bool _deleteFile(const String& dirPath);

		static sl_bool _deleteDirectory(const String& dirPath);

	};
	
	// FilePathSegments is not thread-safe
	class SLIB_EXPORT FilePathSegments
	{
	public:
		sl_uint32 parentLevel;
		CList<String> segments;
	
	public:
		FilePathSegments();

		~FilePathSegments();
	
	public:
		void parsePath(const String& path);

		String buildPath();
	
	};

}

#endif
