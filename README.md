# 🎮 Git Guide — Unreal Engine 5 · C++ · Assets

> Hướng dẫn đầy đủ Git cho dự án game: source code C++, Blueprint, asset UE5.  
> Từ setup → workflow hằng ngày → xử lý sự cố → tối ưu repo.

---

## 📌 Mục lục

- [⚡ Cheatsheet — Lệnh hằng ngày](#-cheatsheet--lệnh-hằng-ngày)
- [🔧 Setup lần đầu](#-setup-lần-đầu)
- [🌿 Branch](#-branch)
- [📦 Staging & Commit](#-staging--commit)
- [⬆️ Push / Pull / Sync](#️-push--pull--sync)
- [🔀 Merge & Rebase](#-merge--rebase)
- [💥 Xử lý Conflict](#-xử-lý-conflict)
- [⏪ Undo / Rollback](#-undo--rollback)
- [🔍 Xem lịch sử & Debug](#-xem-lịch-sử--debug)
- [🎮 Unreal Engine 5 — Workflow đặc thù](#-unreal-engine-5--workflow-đặc-thù)
- [⚙️ C++ trong UE5](#️-c-trong-ue5)
- [🗂️ Asset Management](#️-asset-management)
- [🔥 Dọn repo phình to](#-dọn-repo-phình-to)
- [📦 Git LFS — Asset lớn](#-git-lfs--asset-lớn)
- [🛡️ .gitignore chuẩn UE5](#️-gitignore-chuẩn-ue5)
- [🤝 Làm việc nhóm](#-làm-việc-nhóm)
- [🚨 Lỗi thường gặp & cách fix](#-lỗi-thường-gặp--cách-fix)
- [📐 Quy tắc vàng](#-quy-tắc-vàng)

---

## ⚡ Cheatsheet — Lệnh hằng ngày

```bash
# ══════════════════════════════════════════
#  BẮT ĐẦU NGÀY LÀM VIỆC
# ══════════════════════════════════════════
git pull                              # Lấy code mới nhất từ remote

# ══════════════════════════════════════════
#  CODE XONG → LƯU LẠI
# ══════════════════════════════════════════
git status                            # Kiểm tra file thay đổi
git add .                             # Stage tất cả
git commit -m "[FEAT] Add inventory"  # Commit với message rõ ràng
git push                              # Đẩy lên remote

# ══════════════════════════════════════════
#  BRANCH NHANH
# ══════════════════════════════════════════
git checkout -b feature/ten-tinh-nang # Tạo + chuyển sang branch mới
git checkout dev                       # Chuyển branch
git branch -D ten-branch               # Xóa branch local
```

---

## 🔧 Setup lần đầu

### Cài đặt & kiểm tra

```bash
git --version             # Kiểm tra đã cài chưa (cần >= 2.30)
git config --list         # Xem config hiện tại
```

### Thiết lập thông tin cá nhân

```bash
git config --global user.name  "Ten Cua Ban"
git config --global user.email "email@example.com"

# Đổi editor mặc định sang VS Code (khuyên dùng)
git config --global core.editor "code --wait"

# Đổi tên branch mặc định thành main
git config --global init.defaultBranch main

# Tắt CRLF auto-convert (quan trọng trên Windows với UE5)
git config --global core.autocrlf false
```

### Clone project về máy

```bash
git clone <LINK_REPO>                  # Clone về folder cùng tên repo
git clone <LINK_REPO> ten-folder       # Clone vào folder tự đặt tên
git clone --depth 1 <LINK_REPO>        # Clone shallow (chỉ lấy commit mới nhất, nhanh hơn)
```

### Khởi tạo repo mới từ project có sẵn

```bash
cd <thu-muc-project>
git init
git remote add origin <LINK_REPO>
git add .
git commit -m "[INIT] Initial project setup"
git push -u origin main
```

---

## 🌿 Branch

### Xem branch

```bash
git branch            # Danh sách branch local (* là branch hiện tại)
git branch -r         # Branch trên remote
git branch -a         # Tất cả branch (local + remote)
```

### Tạo & chuyển branch

```bash
git checkout -b ten-branch              # Tạo mới + chuyển sang
git checkout -b feature/X origin/dev   # Tạo từ branch remote cụ thể
git switch ten-branch                   # Chuyển branch (cú pháp mới hơn)
```

### Đổi tên branch

```bash
git branch -m ten-cu ten-moi           # Đổi tên branch local
git push origin --delete ten-cu        # Xóa tên cũ trên remote
git push -u origin ten-moi             # Push tên mới lên remote
```

### Xóa branch

```bash
git branch -d ten-branch               # Xóa local (chỉ khi đã merge)
git branch -D ten-branch               # Xóa local (force — chưa merge cũng xóa)
git push origin --delete ten-branch    # Xóa trên remote
```

### Quy tắc đặt tên branch

| Loại | Prefix | Ví dụ |
|------|--------|-------|
| Tính năng mới | `feature/` | `feature/inventory-system` |
| Bug fix | `fix/` | `fix/crash-on-pickup` |
| Test asset | `test/` | `test/megascans-rocks` |
| Refactor | `refactor/` | `refactor/character-movement` |
| Release | `release/` | `release/v1.2.0` |

---

## 📦 Staging & Commit

### Stage file

```bash
git add .                     # Stage tất cả thay đổi
git add Source/               # Stage cả folder
git add PlayerCharacter.cpp   # Stage file cụ thể
git add *.cpp                 # Stage theo pattern
git add -p                    # Stage từng phần trong file (interactive)

git reset HEAD tenfile        # Bỏ stage một file (giữ thay đổi)
git restore --staged .        # Bỏ stage tất cả
```

### Commit

```bash
git commit -m "message"              # Commit nhanh
git commit --amend -m "message moi"  # Sửa commit message CUỐI (chưa push)
git commit --amend --no-edit         # Thêm file vào commit cuối (chưa push)
git commit -m "message" --allow-empty  # Commit rỗng (dùng để trigger CI)
```

### Format Commit Message — Chuẩn cho UE5

```
[TYPE] Mô tả ngắn gọn (dưới 72 ký tự)
```

| Tag | Dùng khi | Ví dụ |
|-----|----------|-------|
| `[FEAT]` | Thêm tính năng mới | `[FEAT] Add inventory drag & drop` |
| `[FIX]` | Sửa bug | `[FIX] Fix null pointer crash on item pickup` |
| `[REFACTOR]` | Tối ưu code, không đổi logic | `[REFACTOR] Split InventoryComponent into modules` |
| `[ASSET]` | Thêm / cập nhật asset | `[ASSET] Add sword animations pack` |
| `[BLUEPRINT]` | Thay đổi Blueprint | `[BLUEPRINT] Update WBP_Inventory layout` |
| `[DOCS]` | Cập nhật tài liệu | `[DOCS] Update README Git LFS section` |
| `[CHORE]` | Config, build, tooling | `[CHORE] Update .gitignore for UE 5.4` |
| `[PERF]` | Tối ưu hiệu năng | `[PERF] Reduce draw calls in inventory UI` |
| `[TEST]` | Thêm / chạy test | `[TEST] Add unit test for InventoryManager` |
| `[INIT]` | Khởi tạo project/module | `[INIT] Setup multiplayer base framework` |

---

## ⬆️ Push / Pull / Sync

### Pull (lấy code mới)

```bash
git pull                        # Pull branch hiện tại (= fetch + merge)
git pull origin dev             # Pull từ branch dev về branch hiện tại
git pull --rebase               # Pull + rebase thay vì merge (history gọn hơn)
git fetch origin                # Chỉ fetch, chưa merge (an toàn hơn)
git fetch --all                 # Fetch toàn bộ remote
```

### Push (đẩy code lên)

```bash
git push                              # Push branch hiện tại
git push -u origin ten-branch         # Push lần đầu + set upstream
git push origin ten-branch            # Push branch cụ thể
git push origin --force               # Force push (⚠️ cẩn thận)
git push origin --force-with-lease    # Force push AN TOÀN HƠN
git push --tags                       # Push tags
```

> ⚠️ **`--force` vs `--force-with-lease`**  
> `--force` ghi đè remote bất kể có gì.  
> `--force-with-lease` sẽ **từ chối** nếu remote có commit bạn chưa fetch → an toàn hơn trong team.

---

## 🔀 Merge & Rebase

### Merge

```bash
# Merge branch feature vào dev
git checkout dev
git pull
git merge feature/inventory-system
git push
```

### Rebase (history gọn hơn)

```bash
# Rebase branch feature lên đầu dev
git checkout feature/inventory-system
git rebase dev

git rebase --continue    # Tiếp tục sau khi fix conflict
git rebase --skip        # Bỏ qua commit hiện tại
git rebase --abort       # Hủy rebase, quay về trạng thái ban đầu
```

### Squash — Gộp nhiều commit thành 1 trước khi merge

```bash
git rebase -i HEAD~3
# Đổi "pick" thành "squash" (hoặc "s") cho commit muốn gộp
# Giữ "pick" cho commit đầu tiên
```

---

## 💥 Xử lý Conflict

### Conflict xảy ra khi nào?

- Hai người sửa cùng một file, cùng dòng
- Merge / rebase branch lệch quá nhiều

### Cách xử lý — File text (.cpp, .h, .ini)

```bash
git pull            # Bị conflict → Git báo file nào bị

# Mở file conflict → tìm dấu:
# <<<<<<< HEAD        ← code của bạn
# =======
# >>>>>>> origin/dev  ← code từ remote

# Sửa thủ công → giữ lại đúng version → xóa dấu <<<, ===, >>>

git add .
git commit -m "[FIX] Resolve merge conflict in InventoryComponent"
git push
```

### Dùng VS Code làm merge tool

```bash
git config --global merge.tool vscode
git config --global mergetool.vscode.cmd 'code --wait $MERGED'
git mergetool       # Mở VS Code để resolve
```

### Conflict với file Binary / .uasset

`.uasset` là file binary — **không thể merge tay**. Phải chọn 1 trong 2 bên:

```bash
git checkout --ours   Content/UI/WBP_Inventory.uasset    # Giữ version của bạn
git checkout --theirs Content/UI/WBP_Inventory.uasset    # Lấy version remote
git add Content/UI/WBP_Inventory.uasset
git commit -m "[FIX] Keep ours version of WBP_Inventory"
```

> 💡 **Phòng tránh conflict .uasset:** Mỗi người phụ trách Blueprint riêng biệt. Không 2 người cùng sửa 1 file .uasset cùng lúc.

---

## ⏪ Undo / Rollback

### Chưa commit

```bash
git restore tenfile.cpp         # Huỷ thay đổi 1 file (về commit cuối)
git restore .                   # Huỷ tất cả thay đổi chưa commit
git clean -fd                   # Xóa file/folder untracked
git clean -fdn                  # Xem trước sẽ xóa gì (dry-run)
```

### Đã commit nhưng chưa push

```bash
git reset --soft HEAD~1         # Undo commit, GIỮ thay đổi (về staging)
git reset --mixed HEAD~1        # Undo commit, GIỮ thay đổi (về untracked)
git reset --hard HEAD~1         # Undo commit, XÓA thay đổi (mất hết)

git commit --amend              # Sửa commit cuối (message hoặc thêm file)
```

### Đã push rồi

```bash
# Cách 1: Revert (AN TOÀN — tạo commit mới đảo ngược, không xóa history)
git revert <commit_hash>
git push

# Cách 2: Reset + force push (dùng khi cần xóa hẳn, ⚠️ cẩn thận trong team)
git log --oneline
git reset --hard <commit_hash>
git push origin <branch> --force-with-lease
```

### Khôi phục file đã xóa

```bash
git checkout HEAD -- tenfile.cpp          # Lấy lại file từ commit cuối
git checkout <commit_hash> -- tenfile.cpp # Lấy lại file từ commit cụ thể
```

### Reflog — cứu cánh cuối cùng

```bash
git reflog                    # Xem TOÀN BỘ lịch sử di chuyển HEAD
git checkout <hash-reflog>    # Khôi phục về trạng thái đó
```

> 💡 `git reflog` lưu mọi thứ trong 90 ngày. Kể cả sau `reset --hard`, bạn vẫn có thể khôi phục nếu chưa chạy `git gc`.

---

## 🔍 Xem lịch sử & Debug

### Log

```bash
git log                              # Lịch sử đầy đủ
git log --oneline                    # Gọn (1 dòng/commit)
git log --oneline --graph            # Visualize branch graph
git log --oneline -10                # 10 commit gần nhất
git log --author="Ten"               # Lọc theo tác giả
git log --since="2 weeks ago"        # Lọc theo thời gian
git log -- tenfile.cpp               # Lịch sử của 1 file cụ thể
git log --diff-filter=D --name-only  # Tìm file đã bị xóa
```

### Diff

```bash
git diff                        # Thay đổi chưa stage
git diff --staged               # Thay đổi đã stage (sẽ commit)
git diff HEAD~1 HEAD            # So sánh 2 commit liền kề
git diff dev feature/inventory  # So sánh 2 branch
git diff -- tenfile.cpp         # Diff 1 file cụ thể
```

### Tìm commit gây ra bug (Git Bisect)

```bash
git bisect start
git bisect bad                  # Commit hiện tại là bug
git bisect good <commit_hash>   # Commit cũ còn tốt
# Git tự checkout giữa → bạn test → đánh dấu:
git bisect good                 # Nếu commit này ok
git bisect bad                  # Nếu commit này có bug
# Git thu hẹp dần → tìm ra commit gây bug
git bisect reset                # Kết thúc bisect
```

### Tìm file to nhất trong repo

```bash
git rev-list --objects --all \
  | git cat-file --batch-check='%(objecttype) %(objectname) %(objectsize) %(rest)' \
  | grep blob \
  | sort -k3 -n -r \
  | head -20 \
  | awk '{print $3, $4}'
```

---

## 🎮 Unreal Engine 5 — Workflow đặc thù

### Cấu trúc thư mục UE5 & Git

```
MyProject/
├── .git/
├── .gitignore               ← quan trọng
├── .gitattributes           ← quan trọng (LFS + line endings)
├── Config/                  ← ✅ commit (cấu hình project)
├── Content/                 ← ⚠️ cẩn thận (xem Asset Management)
│   ├── Characters/          ← ✅ commit (asset chính của team)
│   ├── UI/                  ← ✅ commit
│   ├── Megascans/           ← ❌ không commit (quá nặng)
│   └── UEFN_Base/           ← ❌ không commit
├── Source/                  ← ✅ commit (toàn bộ C++ source)
│   └── MyProject/
│       ├── *.h
│       ├── *.cpp
│       └── MyProject.Build.cs
├── Plugins/                 ← ⚠️ tùy (plugin tự viết: commit, marketplace: không)
├── Saved/                   ← ❌ không commit (auto-generated)
├── Intermediate/            ← ❌ không commit (build files)
├── Binaries/                ← ❌ không commit (compiled output)
└── MyProject.uproject       ← ✅ commit
```

### Workflow chuẩn mỗi ngày

```bash
# ── Sáng: Bắt đầu ────────────────────────────
git pull
# Mở Unreal Editor → project tự regenerate nếu cần

# ── Làm việc ─────────────────────────────────
# Code C++ trong VS/Rider → compile → test trong Editor
# Tạo / sửa Blueprint, asset trong Editor

# ── Trước khi commit ─────────────────────────
# ĐÓNG Unreal Editor (tránh file lock)
git status
git add Source/
git add Content/Characters/
git add Config/
git diff --staged                # Review lần cuối trước khi commit

# ── Commit & push ─────────────────────────────
git commit -m "[FEAT] Add inventory C++ base class"
git push
```

### Quy trình test asset nặng (không làm bẩn repo)

```bash
# Bước 1: Tạo branch riêng KHÔNG push lên remote
git checkout -b test/megascans-rocks

# Bước 2: Add asset, commit
git add Content/Megascans/
git commit -m "[TEST] Add rock pack for testing"

# Bước 3: Test trong Editor thoải mái

# Bước 4: Xong → quay về branch chính
git checkout dev

# Bước 5: Xóa branch test
git branch -D test/megascans-rocks

# Bước 6: Dọn sạch ổ đĩa
git gc --prune=now --aggressive
```

> ⚠️ **TUYỆT ĐỐI KHÔNG** `git push origin test/...` — nếu lỡ push 10GB lên remote, xóa rất phức tạp (phải dùng `filter-repo`).

---

## ⚙️ C++ trong UE5

### File cần commit

```
Source/
├── MyProject/
│   ├── MyProject.Build.cs          ← ✅ bắt buộc
│   ├── MyProjectGameMode.h         ← ✅
│   ├── MyProjectGameMode.cpp       ← ✅
│   ├── InventoryComponent.h        ← ✅
│   └── InventoryComponent.cpp      ← ✅
└── MyProject.Target.cs             ← ✅ bắt buộc
```

### Workflow thêm class C++ mới

```bash
# Sau khi tạo class mới trong Unreal Editor
# Editor tự tạo .h và .cpp → cần add vào Git ngay

git status
git add Source/MyProject/InventoryComponent.h
git add Source/MyProject/InventoryComponent.cpp
git commit -m "[FEAT] Add InventoryComponent base class"
git push
```

### Sau khi pull có thay đổi C++ / .Build.cs

```bash
git pull

# Nếu .h, .cpp, hoặc .Build.cs thay đổi → cần regenerate project files
# Windows: chuột phải file .uproject → "Generate Visual Studio project files"
```

### Tránh conflict trong C++

```bash
# Kiểm tra ai đang sửa file trước khi động vào
git log --oneline -5 -- Source/MyProject/GameTypes.h

# Dùng branch riêng cho mỗi feature
git checkout -b feature/inventory-stacking
```

---

## 🗂️ Asset Management

### Nguyên tắc — cái gì nên commit

| Asset | Kích thước thường | Nên làm gì |
|-------|-------------------|------------|
| Blueprint `.uasset` | < 1 MB | ✅ Commit bình thường |
| Static Mesh `.uasset` | 1–50 MB | ✅ Commit hoặc Git LFS |
| Texture 4K `.uasset` | 50–500 MB | ⚠️ Dùng Git LFS |
| Level `.umap` | 1–100 MB | ✅ Commit (LFS nếu > 50 MB) |
| Megascans / Bridge | Hàng GB | ❌ Không commit — lưu ngoài |
| Marketplace plugin | Hàng GB | ❌ Không commit — cài lại qua UE |
| Audio `.wav` thô | Lớn | ⚠️ Dùng Git LFS |
| Audio `.ogg` | Nhỏ | ✅ Commit bình thường |

### Lỡ commit asset nặng — 3 cách xử lý

#### Cách 1 — Reset về commit trước (nhanh nhất)

```bash
git log --oneline
# Tìm commit trước khi add asset

git reset --hard <commit_hash>

# Nếu đã push:
git push origin <branch> --force-with-lease
```

#### Cách 2 — Xóa asset, giữ history (đơn giản)

```bash
git rm -r --cached Content/Megascans/    # Xóa khỏi tracking (giữ file local)
# hoặc xóa luôn cả file:
git rm -r Content/Megascans/

git commit -m "[CHORE] Remove heavy Megascans from tracking"
git push
# ❗ Repo vẫn còn lịch sử chứa asset → vẫn nặng → dùng Cách 3 nếu cần
```

#### Cách 3 — Xóa hoàn toàn khỏi lịch sử (chuẩn nhất)

```bash
pip install git-filter-repo

# Xóa folder khỏi TOÀN BỘ lịch sử
git filter-repo --path Content/Megascans --invert-paths

# Push lên remote
git push origin --force --all
git push origin --force --tags
```

> ✔️ Sau bước này repo nhẹ thật sự. Asset biến mất hoàn toàn khỏi history.  
> ⚠️ Notify team: mọi người cần clone lại hoặc `git fetch && git reset --hard origin/<branch>`.

---

## 🔥 Dọn repo phình to

### Kiểm tra dung lượng

```bash
# Xem .git đang nặng bao nhiêu
du -sh .git

# Tìm 20 object lớn nhất trong Git database
git rev-list --objects --all \
  | git cat-file --batch-check='%(objecttype) %(objectname) %(objectsize) %(rest)' \
  | grep blob \
  | sort -k3 -n -r \
  | head -20 \
  | awk '{print $3, $4}'
```

### Dọn rác local

```bash
git gc                              # Dọn rác cơ bản
git gc --prune=now --aggressive     # Dọn triệt để + nén mạnh (chạy lâu hơn)
git remote prune origin             # Xóa remote-tracking branch đã bị xóa trên remote
```

### Hiểu bản chất — cái gì xóa thật

| Hành động | Xóa branch | Xóa history | Giảm dung lượng | Hoàn tác được |
|-----------|:----------:|:-----------:|:---------------:|:-------------:|
| `reset --hard` | ❌ | ❌ | ❌ | ✅ qua reflog |
| `push --force` | ❌ | ⚠️ một phần | ❌ | ✅ nếu còn reflog |
| `filter-repo` | ❌ | ✅ | ✅ | ❌ |
| `gc --prune=now` | ❌ | ✅ rác local | ✅ | ❌ |

> 💡 **`git reset` = giả vờ xóa. `git gc` = xóa thật.**

---

## 📦 Git LFS — Asset lớn

Git LFS lưu file lớn trên server riêng, repo Git chỉ chứa **pointer** nhỏ → repo không bị phình.

### Setup Git LFS

```bash
# Cài LFS (1 lần trên máy)
git lfs install

# Track các loại file lớn của UE5
git lfs track "*.uasset"
git lfs track "*.umap"
git lfs track "*.wav"
git lfs track "*.mp3"
git lfs track "*.png"
git lfs track "*.tga"
git lfs track "*.exr"
git lfs track "*.fbx"
git lfs track "*.abc"

# Commit .gitattributes (LFS config)
git add .gitattributes
git commit -m "[CHORE] Setup Git LFS for UE5 assets"
git push
```

### File `.gitattributes` đầy đủ cho UE5

```gitattributes
# ── Unreal Asset ─────────────────────────────
*.uasset  filter=lfs diff=lfs merge=lfs -text
*.umap    filter=lfs diff=lfs merge=lfs -text

# ── 3D / Animation ───────────────────────────
*.fbx     filter=lfs diff=lfs merge=lfs -text
*.abc     filter=lfs diff=lfs merge=lfs -text
*.obj     filter=lfs diff=lfs merge=lfs -text

# ── Texture ───────────────────────────────────
*.png     filter=lfs diff=lfs merge=lfs -text
*.tga     filter=lfs diff=lfs merge=lfs -text
*.bmp     filter=lfs diff=lfs merge=lfs -text
*.psd     filter=lfs diff=lfs merge=lfs -text
*.exr     filter=lfs diff=lfs merge=lfs -text
*.hdr     filter=lfs diff=lfs merge=lfs -text

# ── Audio ─────────────────────────────────────
*.wav     filter=lfs diff=lfs merge=lfs -text
*.mp3     filter=lfs diff=lfs merge=lfs -text
*.ogg     filter=lfs diff=lfs merge=lfs -text

# ── Video ─────────────────────────────────────
*.mp4     filter=lfs diff=lfs merge=lfs -text
*.mov     filter=lfs diff=lfs merge=lfs -text

# ── Line endings (Windows / Mac / Linux) ──────
*.cpp     text eol=lf
*.h       text eol=lf
*.cs      text eol=lf
*.py      text eol=lf
*.json    text eol=lf
*.ini     text eol=lf
```

### Kiểm tra LFS

```bash
git lfs ls-files                             # Danh sách file đang được track bởi LFS
git lfs status                               # Trạng thái LFS
git lfs migrate info                         # Xem file nào nên chuyển sang LFS
git lfs migrate import --include="*.uasset"  # Chuyển file cũ sang LFS (rewrite history)
```

---

## 🛡️ .gitignore chuẩn UE5

```gitignore
# ════════════════════════════════════════════
#  UNREAL ENGINE 5 — .gitignore
# ════════════════════════════════════════════

# ── Build output ─────────────────────────────
Binaries/
Build/
Intermediate/
DerivedDataCache/

# ── Saved / Cache ────────────────────────────
Saved/
.vs/
.idea/
*.VC.db
*.VC.VC.opendb
*.opensdf
*.sdf
*.suo
*.user
*.sln.docstates

# ── Auto-generated ───────────────────────────
*.sln

# ── Asset marketplace / nặng ─────────────────
Content/Megascans/
Content/MSPresets/
Content/UEFN_Base/

# ── Plugins marketplace ──────────────────────
# (bỏ comment nếu muốn ignore plugins marketplace)
# Plugins/Marketplace/

# ── OS ───────────────────────────────────────
.DS_Store
.DS_Store?
._*
.Spotlight-V100
.Trashes
ehthumbs.db
Thumbs.db
Desktop.ini

# ── Logs ─────────────────────────────────────
*.log
crash.dmp
```

---

## 🤝 Làm việc nhóm

### Cấu trúc branch cho team game

```
main      ──────────────────────────── build release ổn định
  │
  └── dev ────────────────────────────── tích hợp mỗi sprint
        ├── feature/inventory-system ─── merge vào dev khi done
        ├── feature/multiplayer-base ─── merge vào dev khi done
        ├── fix/crash-on-level-load  ─── merge vào dev khi done
        └── test/new-vfx-pack        ─── XÓA sau khi test xong
```

| Branch | Mục đích | Ai push |
|--------|----------|---------|
| `main` | Build release ổn định | Lead / CI only |
| `dev` | Tích hợp feature | Merge request only |
| `feature/*` | Từng tính năng | Dev tự do |
| `fix/*` | Bug fix | Dev tự do |
| `test/*` | Test asset nặng | Dev, **không push remote** |

### Quy trình tạo Pull Request

```bash
# Chuẩn bị branch trước khi tạo PR
git checkout feature/inventory-system
git pull origin dev                     # Cập nhật theo dev mới nhất
git rebase dev                          # Rebase để history gọn
# Giải quyết conflict nếu có
git push origin feature/inventory-system --force-with-lease

# Tạo Pull Request trên GitHub/GitLab
# → assign reviewer → review → approve → merge → xóa branch
```

### Stash — tạm cất code đang dở

```bash
git stash                          # Cất tất cả thay đổi chưa commit
git stash push -m "ten mo ta"      # Cất với tên mô tả
git stash list                     # Xem danh sách stash
git stash pop                      # Lấy lại stash gần nhất (xóa khỏi list)
git stash apply stash@{0}          # Lấy lại stash cụ thể (giữ trong list)
git stash drop stash@{0}           # Xóa 1 stash
git stash clear                    # Xóa tất cả stash
```

### Tag — đánh dấu version release

```bash
git tag                                  # Xem danh sách tag
git tag -a v1.0.0 -m "First release"    # Tạo tag đầy đủ
git push origin v1.0.0                  # Push 1 tag
git push --tags                          # Push tất cả tag
git checkout v1.0.0                      # Checkout về tag cụ thể
git tag -d v1.0.0                        # Xóa tag local
git push origin --delete v1.0.0         # Xóa tag remote
```

---

## 🚨 Lỗi thường gặp & cách fix

### `error: failed to push some refs`

```bash
# Remote có commit mà local chưa có → pull về rồi push lại
git pull --rebase
git push
```

### `Your local changes would be overwritten by merge`

```bash
# Có thay đổi chưa commit khi pull → stash trước
git stash
git pull
git stash pop
```

### `detached HEAD state`

```bash
# HEAD đang trỏ vào commit, không phải branch
git checkout dev                     # Quay về branch bình thường
# Hoặc tạo branch mới từ đây:
git checkout -b feature/ten-moi
```

### `fatal: refusing to merge unrelated histories`

```bash
git pull origin main --allow-unrelated-histories
```

### `.uasset` bị conflict / corrupt sau merge

```bash
# Không thể merge binary → chọn 1 bên
git checkout --ours   Content/Level/MainLevel.umap
git checkout --theirs Content/Level/MainLevel.umap
git add Content/Level/MainLevel.umap
git commit -m "[FIX] Resolve binary conflict in MainLevel"
```

### Push bị reject vì file quá lớn (> 100 MB)

```bash
# Setup Git LFS cho loại file đó
git lfs track "*.uasset"
git add .gitattributes
git commit -m "[CHORE] Track uasset with LFS"
git push
```

### UE5 không load được sau khi pull

```bash
# Nguyên nhân: .Build.cs hoặc .uproject thay đổi
# Fix: Regenerate project files
# Windows: chuột phải file .uproject → "Generate Visual Studio project files"
```

### Lỡ xóa file quan trọng

```bash
git reflog                              # Tìm commit còn file đó
git checkout <commit_hash> -- tenfile   # Khôi phục file
git add tenfile
git commit -m "[FIX] Restore deleted file"
```

---

## 📐 Quy tắc vàng

```
✅  Luôn git pull trước khi bắt đầu code
✅  Commit nhỏ, thường xuyên, message rõ ràng
✅  Mỗi feature / fix = 1 branch riêng
✅  Đóng Unreal Editor trước khi commit (tránh file lock)
✅  Review git diff --staged trước khi commit
✅  Setup .gitignore và .gitattributes từ ngay đầu project
✅  Mỗi người phụ trách Blueprint / .uasset riêng biệt (tránh conflict binary)

❌  Không commit Binaries/, Intermediate/, Saved/
❌  Không commit Megascans, asset marketplace
❌  Không push --force vào main hoặc dev
❌  Không push branch test chứa asset nặng lên remote
❌  Không 2 người cùng sửa 1 file .uasset cùng lúc
❌  Không commit khi code đang lỗi build
```

---

<div align="center">

*UE 5.4 · Git 2.44+ · Cập nhật 2025*

</div>
