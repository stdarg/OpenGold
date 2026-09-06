# OpenGold contributor guidance

- Use RAII for every owned resource in native code.
- Do not represent ownership with raw pointers. Prefer value types and standard
  RAII containers; when pointer semantics are necessary, use `std::unique_ptr`
  by default and `std::shared_ptr` only for genuine shared ownership.
- After updating this repository, commit your changes and push the current branch
  to `origin`, unless the user explicitly asks otherwise. Include only your own
  changes and complete relevant checks before committing.
