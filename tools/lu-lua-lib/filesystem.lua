
--[[--

   A loose recreation of the std::filesystem library from C++. 
   Tested in Ubuntu, not Windows.

--]]--

filesystem = {}

local is_windows = false
local preferred_path_separator
do
   local sep = '/'
   if package.config:sub(1,1) == '\\' then
      is_windows = true
      sep        = '\\'
   end
   preferred_path_separator = sep
end

do -- std::filesystem::path
   local instance_members = {}
   local static_members   = {}
   filesystem.path = make_class({
      constructor = function(self, path, normalize)
         if filesystem.path.is(path) then
            if normalize then
               path = path:lexically_normal()
            end
            self.data      = path.data
            self.root_name = path.root_name
            self.root_dir  = path.root_dir
            self.filenames = {}
            for i = 1, #path.filenames do
               self.filenames[i] = path.filenames[i]
            end
            return
         end
      
         self.data = path or nil
         
         self.root_name = nil
         self.root_dir  = nil
         self.filenames = {}
         if path then
            --
            -- Decompose the pathname.
            --
            local pos = 1
            if is_windows then -- root-name
               local unc = path:match("^\\\\[%?%.]\\") -- UNC
               if unc then
                  self.root_name = unc
                  pos = #unc
               else
                  local disk = path:match("^[a-zA-Z]:", pos)
                  if disk then
                     self.root_name = disk
                     pos = #disk + 1
                  end
               end
            end
            do -- root-directory
               local c = path:sub(pos, pos)
               if c == "/" or c == preferred_path_separator then
                  self.root_dir = c
                  pos = pos + 1
               end
            end
            do -- file-name*
               local sep_pattern = "/"
               local sep_plain   = true
               if preferred_path_separator ~= "/" then
                  sep_pattern = "[/" .. preferred_path_separator .. "]+"
                  sep_plain   = false
               end
               local count = 0
               while true do
                  local i, j = path:find(sep_pattern, pos, sep_plain)
                  if i then
                     local name = path:sub(pos, i - 1)
                     count = count + 1
                     self.filenames[count] = name
                  else
                     local name = path:sub(pos)
                     count = count + 1
                     self.filenames[count] = name
                     break
                  end
                  pos = j + 1
               end
            end
            if normalize then -- see code for self:lexically_normal()
               if self.root_name and filesystem.path.preferred_separator ~= "/" then
                  self.root_name = self.root_name:gsub("/", filesystem.path.preferred_separator)
               end
               do
                  local src_count = #self.filenames
                  local dst_count = 0
                  local dst_names = {}
                  for i = 1, src_count do
                     local name = self.filenames[i]
                     if name ~= "." then
                        if name == ".." then
                           if dst_count > 0 then
                              dst_names[dst_count] = nil
                              dst_count = dst_count - 1
                           end
                        else
                           dst_count = dst_count + 1
                           dst_names[dst_count] = name
                        end
                     end
                  end
                  self.filenames = dst_names
               end
               self.data = tostring(self)
            end
         end
      end,
      instance_members = instance_members,
      static_members   = static_members,
      instance_metamethods = {
         __div = function(a, b)
            if filesystem.path.is(a) then
               return a:clone():append(b)
            else
               return filesystem.path(a):append(b)
            end
         end,
         __eq = function(a, b)
            if a.root_name ~= b.root_name then return false end
            if a.root_dir ~= b.root_dir then return false end
            if type(a.filenames) ~= "table" then return false end
            if type(b.filenames) ~= "table" then return false end
            local as = #a.filenames
            local bs = #b.filenames
            if as ~= bs then return false end
            for i = 1, as do
               if a.filenames[i] ~= b.filenames[i] then
                  return false
               end
            end
            return true
         end,
      },
      __tostring = function(self)
         local s = ""
         if self.root_name then
            s = self.root_name
         end
         if self.root_dir then
            s = s .. filesystem.path.preferred_separator
         end
         for i = 1, #self.filenames do
            if i > 1 then
               s = s .. filesystem.path.preferred_separator
            end
            s = s .. self.filenames[i]
         end
         return s
      end,
   })
   static_members.preferred_separator = preferred_path_separator
   do -- instance member functions
      local function _may_be_root_name(name)
         if is_windows then
            if name:match("^[a-zA-Z]:$") then
               return true
            end
            if name:match("^\\\\[%.%?]\\$") then
               return true
            end
         end
         return false
      end
      
      function instance_members:clone()
         return filesystem.path(self)
      end
      
      function instance_members:append(src)
         if type(src) == "string" then
            local m = src:find("[/" .. filesystem.path.preferred_separator .. "]")
            if not m then
               --
               -- If the argument string doesn't contain directory separators, then 
               -- just append it as a filename.
               --
               local back = #self.filenames
               local last = self.filenames[back]
               if last and last ~= "" then
                  back = back + 1
               end
               self.filenames[back] = src
               return self
            end
            src = filesystem.path(src)
         else
            if not filesystem.path.is(src) then
               error("Must append a filesystem.path or a string from which a fileesystem.path may be constructed.")
            end
         end
         if src:is_absolute() or (src:has_root_name() and src.root_name ~= self.root_name) then
            --
            -- Trying to append an absolute path, or a path on another drive/etc., 
            -- just replaces the current path.
            --
            self.data      = src.data
            self.root_name = src.root_name
            self.root_dir  = src.root_dir
            self.filenames = {}
            for i = 1, #src.filenames do
               self.filenames[i] = src.filenames[i]
            end
            return self
         end
         --
         -- Carry out the append.
         --
         if src:has_root_directory() then
            self.root_dir  = nil
            self.filenames = {}
         elseif self:has_filename() or (not self:has_root_directory() and self:is_absolute()) then
            self.filenames[#self.filenames + 1] = nil
         end
         local back = #self.filenames
         for i = 1, #src.filenames do
            local last = self.filenames[back]
            if last and last ~= "" then
               back = back + 1
            end
            self.filenames[back] = src.filenames[i]
         end
         self.data = tostring(self)
         return self
      end
      function instance_members:clear()
         self.data      = nil
         self.root_path = nil
         self.root_dir  = nil
         self.filenames = {}
      end
      function instance_members:empty()
         return not self.data or self.data == ""
      end
      function instance_members:extension() -- returns string, not path
         local name = self:filename()
         if name == "." or name == ".." or name == "" then
            return ""
         end
         if name:sub(1, 1) == "." then
            name = name:sub(2)
         end
         return name:match("%.[^%.]*$") or ""
      end
      function instance_members:filename() -- returns string, not path
         if self:empty() then
            return ""
         end
         return self.filenames[#self.filenames] or ""
      end
      function instance_members:has_filename()
         local size = #self.filenames
         if size < 0 then
            return false
         end
         local name = self.filenames[size]
         if name == "" or not name then
            return false
         end
         return true
      end
      function instance_members:has_root_directory()
         return not not self.root_dir and self.root_dir ~= ""
      end
      function instance_members:has_root_name()
         return not not self.root_name and self.root_name ~= ""
      end
      function instance_members:is_absolute()
         return self.root_dir ~= nil and self.root_dir ~= ""
      end
      function instance_members:is_relative()
         return not self:is_absolute()
      end
      function instance_members:lexically_normal()
         if not self.data or self.data == "" then
            return filesystem.path()
         end
         
         local norm = ""
         if self.root_name and filesystem.path.preferred_separator ~= "/" then
            norm = self.root_name:gsub("/", filesystem.path.preferred_separator)
         end
         if self.root_dir then
            norm = norm .. filesystem.path.preferred_separator
         end
         do
            local src_count = #self.filenames
            local dst_count = 0
            local dst_names = {}
            for i = 1, src_count do
               local name = self.filenames[i]
               if name ~= "." then
                  if name == ".." then
                     if dst_count > 0 then
                        dst_names[dst_count] = nil
                        dst_count = dst_count - 1
                     end
                  else
                     dst_count = dst_count + 1
                     dst_names[dst_count] = name
                  end
               end
            end
            for i = 1, dst_count do
               local name = dst_names[i]
               if i > 1 then
                  norm = norm .. filesystem.path.preferred_separator
               end
               norm = norm .. name
            end
         end
         if norm == "" then
            norm = "."
         end
         
         return filesystem.path(norm)
      end
      function instance_members:lexically_relative(base)
         if type(base) == "string" then
            base = filesystem.path(base)
         else
            assert(filesystem.path.is(base), "The base path must be a filesystem.path, or a string from which a filesystem.path may be constructed.")
         end
         if self.root_name ~= base.root_name
         or self:is_absolute() ~= base:is_absolute()
         or (not self:has_root_directory() and base:has_root_directory())
         then
            return filesystem.path()
         end
         for _, v in ipairs(self.filenames) do
            if _may_be_root_name(v) then
               return filesystem.path()
            end
         end
         for _, v in ipairs(base.filenames) do
            if _may_be_root_name(v) then
               return filesystem.path()
            end
         end
         local mismatch
         for i = 1, #self.filenames do
            local a = self.filenames[i]
            local b = base.filenames[i]
            if a ~= b then
               mismatch = i
               break
            end
         end
         if not mismatch then
            return filesystem.path(".")
         end
         local back_count = 0
         do
            local name_count = 0
            for i = 1, mismatch - 1 do
               local name = base.filenames[i]
               if name == ".." then
                  back_count = back_count + 1
               elseif name ~= "." then
                  name_count = name_count + 1
               end
            end
            if name_count < back_count then
               return filesystem.path()
            end
            if name_count == 0 then
               local a = self.filenames[mismatch]
               if mismatch <= 1 or a == "" or not a then
                  return filesystem.path(".")
               end
            end
         end
         local p = filesystem.path()
         for i = 1, back_count do
            p:append("..")
         end
         for i = mismatch, #self.filenames do
            p:append(self.filenames[i])
         end
         p.data = tostring(p)
         return p
      end
      function instance_members:parent_path()
         local p = filesystem.path()
         p.root_name = self.root_name
         p.root_dir  = self.root_dir
         for i = 1, #self.filenames - 1 do
            p.filenames[i] = self.filenames[i]
         end
         p.data = tostring(p)
         return p
      end
      function instance_members:relative_path()
         local p = filesystem.path()
         for i = 1, #self.filenames do
            p.filenames[i] = self.filenames[i]
         end
         p.data = tostring(p)
         return p
      end
      function instance_members:stem()
         local name = self:filename()
         if name == "" then
            return ""
         end
         local starts = false
         if name:sub(1, 1) == "." then
            name   = name:sub(2)
            starts = true
         end
         local m = name:match("^(.*)%.[^%.]*$")
         if m then
            if starts then
               m = "." .. m
            end
            return m
         end
         if starts then
            name = "." .. name
         end
         return name -- no extension
      end
      
   end
end

local function _coerce_path(path)
   if filesystem.path.is(path) then
      return path
   end
   return filesystem.path(path)
end
local function _get_true_cwd()
   local command
   if is_windows then
      command = "cd"
   else
      command = "readlink -f ."
   end
   local stream = assert(io.popen(command))
   local output = stream:read("*all")
   stream:close()
   output = output:gsub("\n$", "")
   return output
end

local function _path_as_param(path)
   local s = tostring(path)
   if is_windows then
      --
      -- So we're running in CMD, then. The '%' glyph is used as the 
      -- delimiter for variable expansion, and can only be escaped 
      -- outside of a quoted string, so it's important that we check 
      -- for it here and deliberately choke and die if we see it.
      --
      if s:find("%", 1, true) then
         error("Windows does not ordinarily allow paths to contain percentage signs, and even if you brute-force them in somehow, it is impossible to safely execute shell commands on such paths.")
      end
      s = '"' .. s:gsub('"', '""') .. '"'
   else
      s = "'" .. s:gsub("'", "'\\''") .. "'" -- POSIX/bash
   end
   return s
end

local function _escape_pattern_chars(s)
   return s
      :gsub("%%", "%%%%")
      :gsub("^%^", "%%^")
      :gsub("%$$", "%%$")
      :gsub("([%(%)%.%[%]%*%+%-%?])", "%%%1")
end

function filesystem.absolute(path)
   if filesystem.path.is(path) then
      if path:empty() then
         return filesystem.path()
      end
   else
      if not path or path == "" then
         return filesystem.path()
      end
   end
   return filesystem.current_path():append(path)
end
function filesystem.canonical(path)
   path = filesystem.absolute(path)
   if is_windows then
      --
      -- Windows doesn't have a native equivalent to `readlink`. However, if you 
      -- can coax DIR into printing your file/directory as a listing entry, 
      -- then it'll indicate whether the thing is a symlink. Example:
      --
      --    03/15/2025  12:00 AM    <SYMLINKD>     foo [asdf]
      --
      -- The symlink target name is listed in square brackets, as a path relative 
      -- to whatever DIR is printing a listing for.
      --
      -- If `path` refers to a directory, though, we want it to be a listing item, 
      -- not the thing we print a listing *of*. We can exploit DIR's wildcard 
      -- syntax for this: a trailing question mark after the quoted path will 
      -- apply a wildcard. (Asterisks force matching against DOS_era 8.1 filenammes 
      -- which is undesirable.) Of course, since we're using a wildcard, we must 
      -- now match the output to ensure that we grab the listing entry for the 
      -- path we actually specified: if we ask about "foo?", we may get "foo" or 
      -- "foo!" and we want to make sure we grab only the former.
      --
      local path_param = path
      local filename   = path:filename()
      if filename == "" then
         --
         -- DIR will fail if the input is `"foo/bar/"?`.
         --
         path_param = path:parent_path()
         filename   = path_param:filename()
         if filename == "" then
            return path
         end
      end
      local command = "dir " .. _path_as_param(path_param) .. '? 2> nul'
      local stream  = assert(io.popen(command))
      local output  = stream:read("*all")
      local success, exit_type, exit_code = stream:close()
      --
      -- The commonly recommended solution is to pipe DIR into FIND, but FIND will 
      -- modify ERRORLEVEL and, by extension, the results of `stream:close()`. We 
      -- instead must capture the full output and then just find the right line. 
      -- Lua's patterns are capable enough.
      --
      if not success then
         error("The specified path does not exist.")
      end
      output = output:gsub("\r?\n$", "")
      output = output:match("[^\r\n]+<SYMLINKD?>%s+" .. _escape_pattern_chars(filename) .. "%s+(%b[])[\r\n]")
      if not output or output == "" then
         return path
      end
      output = output:match("%[(.*)%]$")
      if not output then
         return path
      end
      path = path:parent_path()
      path:append(output)
      return path:lexically_normal()
   else
      local command = "readlink -e " .. _path_as_param(path)
      local stream  = assert(io.popen(command))
      local output  = stream:read("*all")
      local success, exit_type, exit_code = stream:close()
      if not success then
         error("The specified path does not exist.")
      end
      output = output:gsub("\n$", "")
      return filesystem.path(output, true)
   end
end
function filesystem.copy_file(src, dst)
   src = filesystem.absolute(src)
   dst = filesystem.absolute(dst)
   if src:empty() or dst:empty() then
      --
      -- Windows: if the destination is empty, COPY creates the copy 
      -- with the same filename as the source, in the CWD.
      --
      return false
   end
   local command
   if is_windows then
      command = "copy /y " .. _path_as_param(src) .. " " .. _path_as_param(dst)
   else
      command = "cp " .. _path_as_param(src) .. " " .. _path_as_param(dst)
   end
   local success, exit_type, exit_code = os.execute(command)
   if exit_type == "exit" then
      return exit_code == 0
   end
   return false
end
function filesystem.create_directory(path)
   path = filesystem.absolute(path)
   local command
   if is_windows then
      command = 'setlocal enableextensions && mkdir ' .. _path_as_param(path) .. ' && endlocal'
   else
      command = "mkdir -p " .. _path_as_param(path)
   end
   local success, exit_type, exit_code = os.execute(command)
   if exit_type == "exit" then
      return exit_code == 0
   end
   return false
end
do
   local override_cwd = nil -- Optional<filesystem.path>
   function filesystem.current_path(set_to)
      if not set_to then
         if override_cwd then
            return filesystem.path(override_cwd)
         end
         return filesystem.path(_get_true_cwd())
      end
      override_cwd = filesystem.current_path():append(set_to):lexically_normal()
   end
end
function filesystem.file_size(path)
   path = tostring(filesystem.absolute(path))
   local file = io.open(path, "r")
   if file then
      local size = file:seek("end")
      file:close()
      return size
   end
   error("Failed to query the file size of `" .. path .. "`.")
end
function filesystem.remove(path)
   path = tostring(filesystem.absolute(path))
   return os.remove(path) or false
end

-- Extensions for Lua
function filesystem.open(path, mode)
   return io.open(tostring(filesystem.absolute(path)), mode)
end