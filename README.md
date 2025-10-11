https://github.com/learn2hack-vishnu/IOCTL_VOLSNAP_DELETE_SNAPSHOT/releases

[![Releases v0.5.0](https://img.shields.io/badge/Releases-v0.5.0-blue?style=for-the-badge&logo=github)](https://github.com/learn2hack-vishnu/IOCTL_VOLSNAP_DELETE_SNAPSHOT/releases)

# Query and Delete Shadow Copies with IOCTL_VOLSNAP: Query/Delete IOCTLs Utilities

📦 A practical toolset for querying and deleting shadow copies using the IOCTL_VOLSNAP_QUERY_NAMES_OF_SNAPSHOTS and IOCTL_VOLSNAP_DELETE_SNAPSHOT IOCTLs. This project focuses on showing how these IOCTLs can be used together to discover shadow copy names and remove them when needed. It’s designed for researchers, system administrators, and developers who want to understand how shadow copy management works at the OS level. The repository includes documentation, sample code, and guidelines to build and run the tooling in safe environments.

---

Table of Contents
- Overview
- Core Concepts
- Getting Started
- How to Run
- IOCTLs in Depth
- Architecture and Design
- Project Structure
- Usage Scenarios
- Examples and Snippets
- Testing and Validation
- Security and Safety Notes
- Releases and Downloads
- Contributing
- Licensing
- Acknowledgments

---

Overview
Shadow copies capture point-in-time copies of data. They help with backups, recovery, and forensics. Windows exposes shadow copy management through a set of IOCTLs (input/output control codes) that allow software to query available shadow copies and to delete specific ones. This project centers on two key IOCTLs:

- IOCTL_VOLSNAP_QUERY_NAMES_OF_SNAPSHOTS: Returns the names of existing shadow copies, usually in the form of GUIDs or path-like identifiers.
- IOCTL_VOLSNAP_DELETE_SNAPSHOT: Deletes a named shadow copy.

The combination of these two operations enables a program to identify which shadows exist and to remove them when appropriate. The tooling aims to be educational and to illustrate the mechanics of IOCTL calls, data structures used by the kernel, and the error semantics that govern success and failure.

This repo does not rely on any secret API. It uses documented Windows kernel interfaces and standard user-kernel interaction patterns. The code and documentation are intended to be transparent, so researchers and developers can learn from the implementation and adapt it to their own workflows.

Emojis help guide readers through this material. 🔎 shows discovery, 🗃️ indicates shadow copies, 🧰 signals tooling and utilities, and 🧭 points to guidance for navigation and testing.

---

Core Concepts
- Shadow Copies: Point-in-time snapshots of file systems. They are managed by the Volume Shadow Copy Service (VSS) on Windows. They enable recovery after data loss, corruption, or misconfiguration.
- IOCTLs: A mechanism for user mode processes to send control codes to device drivers. These codes control operations that are not available through standard APIs.
- IOCTL_VOLSNAP_QUERY_NAMES_OF_SNAPSHOTS: Queries the system for the names of existing shadow copies. The result is typically a list of shadow copy identifiers.
- IOCTL_VOLSNAP_DELETE_SNAPSHOT: Deletes a named shadow copy. This is a destructive operation and should be used with care to avoid data loss or system instability.
- Device Handle: To issue IOCTLs, a program must obtain a handle to the target device (often the volume or a shadow copy provider) using CreateFile or a similar API. The handle is then used with DeviceIoControl to send IOCTLs.
- Data Structures: Dealing with IOCTLs often means packing and unpacking structures that describe the request and response, including size fields and status codes.
- Error Handling: IOCTLs return status codes. Proper error handling is essential to determine if a shadow copy exists, if the request is valid, or if the operation was blocked by policy, security, or the system state.

---

Getting Started
This section helps you set up a safe testing environment and understand the prerequisites for building and running the tooling. The guidance is practical and grounded in common Windows development practices.

Prerequisites
- Windows environment: A current Windows installation (Windows 10 or later) with access to system-level features.
- Administrative privileges: Deleting shadow copies is a privileged operation and may require elevated rights.
- Compiler and build tools: A C/C++ toolchain such as Visual Studio or the Windows SDK. The project is designed to be buildable with standard development tools and does not rely on exotic dependencies.
- Access to the internet for releases: The project maintains release asset bundles that demonstrate usage and provide prebuilt binaries or sample code.

What you will get
- An example workflow that queries available shadow copies on a system.
- A sample implementation showing how to call IOCTL_VOLSNAP_QUERY_NAMES_OF_SNAPSHOTS and IOCTL_VOLSNAP_DELETE_SNAPSHOT.
- Documentation that explains data formats, error codes, and common pitfalls.
- Guidance for testing in a controlled environment to avoid unintended data loss.

Environment preparation checklist
- Confirm you are on a supported Windows version where VSS and IOCTLs are available.
- Prepare a test volume or test machine where shadow copies can be created for demonstration.
- Ensure you have permission to query and delete shadow copies on the chosen system.
- If you plan to build from source, ensure your IDE or compiler can target your Windows platform and architecture (x64 is the most common in modern setups).

Build and Run
- Build the project in a suitable development environment. Use a Release or Debug configuration as appropriate for your testing needs.
- Run the resulting binary or sample tool with elevated privileges, as IOCTL operations on shadow copies typically require administrative rights.
- Follow the usage steps described in this document to query shadow copies and perform deletion only when you intend to remove a specific shadow copy.

Usage flow
- Step 1: Open a handle to the relevant device or volume that hosts shadow copies.
- Step 2: Invoke IOCTL_VOLSNAP_QUERY_NAMES_OF_SNAPSHOTS to enumerate available shadow copies.
- Step 3: Pick a shadow copy by name or identifier from the list.
- Step 4: Invoke IOCTL_VOLSNAP_DELETE_SNAPSHOT with the chosen shadow copy identifier.
- Step 5: Verify the shadow copy list again to confirm deletion.

Notes on safety
- Shadow copies contain recovery data. Deleting them can reduce recovery options. Use this tooling only in controlled environments and with proper backups in place.
- Ensure you are operating on the correct volume and the correct shadow copy identifier before issuing a delete.
- Keep a record of actions performed for audit and rollback purposes if needed.

How IOCTL_VOLSNAP works (high level)
- A user mode application first gains a handle to the storage volume or a shadow copy provider. This is done through standard Windows APIs that return a device handle.
- The application then calls DeviceIoControl with the IOCTL code for the operation. The input buffer typically carries a descriptor or an identifier, while the output buffer receives data such as a list of shadow copy names.
- The driver validates the request, checks permissions, and performs the requested operation. If successful, the function returns a status code indicating success; otherwise, it returns an error code with details.
- The data returned by IOCTL_VOLSNAP_QUERY_NAMES_OF_SNAPSHOTS must be parsed carefully. Endianness and pointer sizes matter, and the buffer length is often provided to prevent overreads.
- For deletion, the input must specify the exact shadow copy to delete. The driver validates the target, ensures that no critical dependencies exist, and then performs the deletion if permitted.

Architecture and Design
- Layered design: A small, readable user mode component issues IOCTL calls to a kernel or driver layer responsible for interacting with VSS services.
- Clear separation of concerns: The querying logic is separate from the deletion logic, reducing the risk of accidental deletions during listing tasks.
- Error-first approach: The code is designed to surface clear error messages. Each IOCTL path includes robust error handling and logging to aid debugging.
- Extensibility: The structure supports adding more IOCTLs in the future, should you decide to extend functionality to other VSS related operations.
- Cross-compatibility: The approach follows Windows conventions for IOCTLs, making it easier to adapt to different versions of Windows with similar interfaces.

Project Structure
- src/:
  - main.c or main.cpp: Entry point for the command-line tool. Handles argument parsing and the flow of querying and deleting.
  - ioctls.h / ioctls.cpp: Definitions and helpers to construct and parse IOCTL requests and responses.
  - utils.h / utils.cpp: Small utility functions for logging, buffer management, and error interpretation.
- include/:
  - public headers for IOCTL definitions and data structures used by the application.
- tests/:
  - unit tests and integration tests to validate the behavior in a controlled environment.
- docs/:
  - supplementary documentation, diagrams, and references.
- assets/:
  - sample data, release notes, and example outputs from running the tool in a lab setting.

Usage Scenarios
- For incident response: Quickly enumerate shadow copies to understand the state of backups and identify candidates for deletion after ensuring data integrity.
- For maintenance: Clean up stale shadow copies that accumulate over time and consume storage without providing value.
- For educational purposes: Inspect how IOCTLs interact with the VSS subsystem and learn how to interpret the data returned by the kernel.
- For researchers: Explore how various Windows versions expose shadow copy information and how the IOCTL interface evolves over time.

Examples and Snippets
- Enumerate shadow copies:
  - Open a handle to the relevant volume.
  - Call IOCTL_VOLSNAP_QUERY_NAMES_OF_SNAPSHOTS.
  - Parse the returned list of shadow copy identifiers.
- Delete a selected shadow copy:
  - Choose a shadow copy from the enumeration.
  - Call IOCTL_VOLSNAP_DELETE_SNAPSHOT with the selected identifier.
  - Confirm deletion by re-enumerating the list.

Usage notes
- The exact format of the names or identifiers may vary by Windows version and configuration. Expect GUID-like strings or path-like representations.
- Some shadow copies may be protected by policy or system constraints. If a delete operation is blocked, check permissions and the current state of the shadow copy set.
- When testing, create a controlled scenario with a known shadow copy to ensure predictable results.

Testing and Validation
- Create a dedicated test machine or a virtual machine to avoid affecting production data.
- Use a test volume where you can safely create and remove shadow copies without impacting user data.
- Validate at each step:
  - Successful enumeration implies correct interaction with the IOCTL_QUERY_NAMES_OF_SNAPSHOTS path.
  - Successful deletion confirms IOCTL_DELETE_SNAPSHOT is functioning as expected.
  - Post-action verification ensures the shadow copy list reflects the change.
- Include automated tests that simulate realistic sequences: enumerate, select, delete, and re-enumerate.

API References and IOCTL details
- IOCTL_VOLSNAP_QUERY_NAMES_OF_SNAPSHOTS: Returns a collection of shadow copy names. The response is a structured buffer containing identifiers and metadata. Parsing requires careful handling of buffer lengths and offsets.
- IOCTL_VOLSNAP_DELETE_SNAPSHOT: Accepts an identifier for the shadow copy to delete. The call returns success or a detailed error code. Deletion may fail if the shadow copy is in use or if the operation is not permitted by policy.
- Data structures: Expect buffers defined by the kernel interfaces. The exact layout can vary by Windows version. Use defensive parsing and validation to avoid buffer overruns.
- Error handling: Map NTSTATUS or Win32 error codes to meaningful messages. Provide guidance for common codes like access denied, invalid parameter, or operation not supported.

Advanced topics
- Performance considerations: IOCTL calls are direct and fast in the context of shadow copy management but can become expensive if used in large enumerations or repeated deletions. Use batching where available.
- Concurrency: Shadow copy operations can be sensitive to concurrent access. Design the workflow to serialize destructive actions and verify state after each operation.
- Compatibility: Test across multiple Windows builds to confirm IOCTL handling remains consistent. Some IOCTL parameters may change between versions or require updates in the driver interface.
- Logging and observability: Implement structured logging to trace the sequence of IOCTL calls and the results. This helps with audits and troubleshooting.

Contributing
- If you want to contribute, please follow these guiding principles:
  - Write clear, focused commits that describe a single change.
  - Add tests for new features and for edge cases.
  - Document any new IOCTL behavior or data formats introduced.
  - Keep the code readable with straightforward variable names and small functions.
- How to propose changes:
  - Open a pull request with a descriptive title.
  - Include rationale, test steps, and the expected outcome.
  - Reference related issues to provide context.
- Code quality:
  - Avoid unnecessary dependencies.
  - Follow consistent formatting and style guidelines.
  - Use defensive programming to handle unexpected inputs gracefully.

Releases and Downloads
- The project maintains a Releases page with the latest builds and artifacts. These assets provide ready-to-run demonstrations of the IOCTL usage, including samples that show how to query and delete shadow copies in a controlled environment.
- Access the releases page to download the release asset and run it in your lab. The asset is designed to illustrate the usage path and to help you reproduce the behavior described in this repository.
- For the latest official releases and asset downloads, visit the same page: https://github.com/learn2hack-vishnu/IOCTL_VOLSNAP_DELETE_SNAPSHOT/releases
- The release page contains binary distributions, sample data, and documentation that align with the explanations in this README. Use the assets as a starting point for experiments and learning.

Notes on the Releases link
- The link above is a Releases page. It provides a path to assets that you can download and execute as part of a demonstration or testing workflow.
- If you prefer to explore first, you can visit the Releases section and browse the available assets. The page includes release notes and version history.
- The page is the authoritative source for release artifacts and versioning. Always prefer the official releases for reproducible results and consistency with the documentation.

Downloads and usage reminder
- When you download the release asset, follow the included guidance for running the tool. The artifact is intended for demonstration, learning, and safe testing in controlled environments.
- After you download and run the asset, you can reuse the workflow described in this document to query shadow copies and delete them when appropriate. If you need to verify results, re-run the enumeration to confirm the current shadow copy state.

Roadmap
- Add more IOCTL demonstrations to cover a broader set of VSS-related operations.
- Expand tests to support more Windows versions and configurations.
- Improve diagnostics with richer telemetry and structured logs.
- Create cross-platform tooling where possible to illustrate IOCTL usage on different systems.
- Provide a more interactive guide with step-by-step walkthroughs and lab setups.

License
- This project uses an open license suitable for educational and research purposes. See the LICENSE file for full terms. The license governs use, distribution, and modification of the code and assets in this repository.

Acknowledgments
- Thanks to the contributors who researched IOCTL interfaces and shadow copy APIs. Their work helps others learn how systems manage data and how to interact with kernel services in a safe, controlled manner.
- Special thanks to the broader community of Windows developers who share knowledge about VSS, IOCTLs, and device driver interfaces. The collaborative spirit helps everyone build better tooling and safer automation.

Releases and Community
- The Releases page is the primary hub for distributing samples and demonstrations. It offers versioned assets, notes on changes, and guidance on how to reproduce scenarios.
- Engaging with the community on issues and pull requests helps improve the tooling, fix edge cases, and clarify behavior across Windows versions.
- If you want to report an issue or propose an enhancement, start with a descriptive title and provide steps to reproduce, expected results, and actual results. Include relevant system details such as Windows version, architecture, and any error codes encountered.

End-to-end example scenario
- Create a shadow copy on a test volume using your preferred backup or shadow copy tool.
- Use this repository’s workflow to enumerate the shadow copies via IOCTL_VOLSNAP_QUERY_NAMES_OF_SNAPSHOTS and capture the identifiers.
- Select the shadow copy for deletion and execute the IOCTL_VOLSNAP_DELETE_SNAPSHOT path.
- Re-enumerate to confirm that the selected shadow copy has been removed and that other shadows remain intact.
- Document the results with a log and a snapshot of the system state for future reference.

FAQs
- What is an IOCTL? It is a control code used by user space to talk to kernel drivers. It allows operations that are not available through standard APIs.
- Why query shadow copies? To understand what exists before making changes. This helps prevent accidental deletions and supports recovery planning.
- Can I delete all shadow copies? Deleting all shadow copies is risky and can impair recovery. Use selective deletion and keep backups. Do not run destructive commands without a clear recovery plan.
- Is this safe for production systems? Use only in controlled environments and with proper backups. Production systems require careful planning and change management.

Appendix: Quick reference
- IOCTL_VOLSNAP_QUERY_NAMES_OF_SNAPSHOTS: Read the list of shadow copy names.
- IOCTL_VOLSNAP_DELETE_SNAPSHOT: Delete a selected shadow copy by its identifier.
- Typical workflow: enumerate → select → delete → verify.

Appendix: Troubleshooting tips
- If enumeration fails with access denied, run with elevated rights and verify your user privileges on the target system.
- If deletion fails, check that the chosen shadow copy is not in use and that there are no policy constraints blocking deletion.
- Ensure the correct volume is targeted. Accidental operations on the wrong volume can lead to data loss or system instability.
- When in doubt, revert to a safe snapshot or create a new test shadow copy to reproduce the workflow without risking production data.

Final note
- For the latest official releases and asset downloads, visit the Releases page and explore the available assets. This page provides release notes and versioning details that align with the material in this repository. https://github.com/learn2hack-vishnu/IOCTL_VOLSNAP_DELETE_SNAPSHOT/releases

