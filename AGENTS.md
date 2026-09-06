# OpenGold contributor guidance

- Use RAII for every owned resource in native code.
- Do not represent ownership with raw pointers. Prefer value types and standard
  RAII containers; when pointer semantics are necessary, use `std::unique_ptr`
  by default and `std::shared_ptr` only for genuine shared ownership.
