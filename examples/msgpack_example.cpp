// Glaze Library - MessagePack Example
// For the license information refer to glaze.hpp

#include <iostream>
#include <vector>
#include <map>

#include "glaze/glaze.hpp"
#include "glaze/msgpack.hpp"

// Define a simple struct
struct person
{
   std::string name = "John Doe";
   int age = 30;
   std::vector<std::string> hobbies = {"reading", "hiking", "coding"};
   std::map<std::string, double> scores = {{"math", 95.5}, {"science", 88.0}};
};

// Add glaze metadata for the struct
template <>
struct glz::meta<person>
{
   using T = person;
   static constexpr auto value = object("name", &T::name, "age", &T::age, "hobbies", &T::hobbies, "scores", &T::scores);
};

int main()
{
   std::cout << "=== Glaze MessagePack Serialization Example ===" << std::endl << std::endl;

   // Create a person object
   person p;
   p.name = "Alice Smith";
   p.age = 25;
   p.hobbies = {"painting", "photography", "travel"};
   p.scores = {{"english", 92.0}, {"history", 87.5}, {"art", 96.0}};

   std::cout << "Original person:" << std::endl;
   std::cout << "  Name: " << p.name << std::endl;
   std::cout << "  Age: " << p.age << std::endl;
   std::cout << "  Hobbies: ";
   for (const auto& hobby : p.hobbies) {
      std::cout << hobby << " ";
   }
   std::cout << std::endl;
   std::cout << "  Scores: ";
   for (const auto& [subject, score] : p.scores) {
      std::cout << subject << "=" << score << " ";
   }
   std::cout << std::endl << std::endl;

   // Serialize to MessagePack
   std::string msgpack_buffer;
   auto ec = glz::write_msgpack(p, msgpack_buffer);
   if (ec) {
      std::cerr << "Failed to serialize to MessagePack: " << glz::format_error(ec, msgpack_buffer) << std::endl;
      return 1;
   }

   std::cout << "Serialized to MessagePack (" << msgpack_buffer.size() << " bytes)" << std::endl;

   // Also serialize to JSON for comparison
   std::string json_buffer;
   ec = glz::write_json(p, json_buffer);
   if (ec) {
      std::cerr << "Failed to serialize to JSON" << std::endl;
      return 1;
   }

   std::cout << "Serialized to JSON (" << json_buffer.size() << " bytes)" << std::endl;
   std::cout << "JSON: " << json_buffer << std::endl;
   std::cout << "MessagePack is " << (100.0 * (json_buffer.size() - msgpack_buffer.size()) / json_buffer.size())
             << "% more compact than JSON" << std::endl
             << std::endl;

   // Deserialize from MessagePack
   person p2;
   ec = glz::read_msgpack(p2, msgpack_buffer);
   if (ec) {
      std::cerr << "Failed to deserialize from MessagePack: " << glz::format_error(ec, msgpack_buffer) << std::endl;
      return 1;
   }

   std::cout << "Deserialized from MessagePack:" << std::endl;
   std::cout << "  Name: " << p2.name << std::endl;
   std::cout << "  Age: " << p2.age << std::endl;
   std::cout << "  Hobbies: ";
   for (const auto& hobby : p2.hobbies) {
      std::cout << hobby << " ";
   }
   std::cout << std::endl;
   std::cout << "  Scores: ";
   for (const auto& [subject, score] : p2.scores) {
      std::cout << subject << "=" << score << " ";
   }
   std::cout << std::endl << std::endl;

   // Verify data integrity
   bool success = (p.name == p2.name) && (p.age == p2.age) && (p.hobbies == p2.hobbies) && (p.scores == p2.scores);

   if (success) {
      std::cout << "✓ Round-trip successful! Data integrity verified." << std::endl;
   }
   else {
      std::cout << "✗ Round-trip failed! Data mismatch detected." << std::endl;
      return 1;
   }

   return 0;
}
