#include "pch.h"
#include "TestHelpers.h"
#define CIRCULAR_BUFFER_STRESS_TEST 0

namespace ECSTests
{
	TEST(TypeTests, ComponentTypeTest)
	{
		using namespace ECS;
		ComponentTypeID transformType = ComponentType<Transform>();
		ComponentTypeID aType = ComponentType<A>();
		ComponentTypeID bType = ComponentType<B>();
		EXPECT_EQ(aType, ECS::ComponentType<A>());
		EXPECT_EQ(bType, ECS::ComponentType<B>());
		EXPECT_EQ(transformType, ECS::ComponentType<Transform>());
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// CircularBuffer Tests ////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////

	TEST(CircularBufferTests, CreateCircularBuffer)
	{
		using namespace ECS;
		CircularBuffer<int> buffer;

		EXPECT_EQ(buffer.GetCapacity(), 16);

		CircularBuffer<int> customBuffer(10);
		EXPECT_EQ(customBuffer.GetCapacity(), 10);
	}

	TEST(CircularBufferTests, CircularBufferCount)
	{
		using namespace ECS;
		CircularBuffer<int> buffer(3);

		EXPECT_EQ(buffer.GetSize(), 0);
		EXPECT_NE(buffer.IsEmpty(), false);
		EXPECT_EQ(buffer.IsEmpty(), true);
		EXPECT_EQ(buffer.IsFull(), false);

		buffer.PushBack(1);
		buffer.PushBack(2);
		buffer.PushBack(3);

		EXPECT_EQ(buffer.GetSize(), 3);
		EXPECT_EQ(buffer.IsEmpty(), false);
		EXPECT_EQ(buffer.IsFull(), true);
	}

	TEST(CircularBufferTests, CircularBufferCreationStressTest)
	{
		using namespace ECS;
		CircularBuffer<int> buffer(10000);

		EXPECT_EQ(buffer.GetCapacity(), 10000);
		EXPECT_EQ(buffer.GetSize(), 0);
		EXPECT_EQ(buffer.IsEmpty(), true);

		for (int i = 0; i < 10000; i++)
		{
			buffer.PushBack(i);
		}

		EXPECT_EQ(buffer.GetCapacity(), 10000);
		EXPECT_EQ(buffer.GetSize(), 10000);
		EXPECT_EQ(buffer.IsEmpty(), false);
	}

	TEST(CircularBufferTests, CircularBufferAccessors)
	{
		using namespace ECS;
		CircularBuffer<int> buffer(3);

		buffer.PushBack(1);
		buffer.PushBack(2);
		buffer.PushBack(3);

		EXPECT_EQ(buffer.GetFront(), 1);
		EXPECT_EQ(buffer.GetBack(), 3);
		EXPECT_EQ(buffer[0], 1);
		EXPECT_EQ(buffer[1], 2);
		EXPECT_EQ(buffer[2], 3);
	}

	TEST(CircularBufferTests, CircularBufferResize)
	{
		using namespace ECS;
		CircularBuffer<int> buffer(3);
		EXPECT_EQ(buffer.GetCapacity(), 3);
		for (int i = 0; i < 3; ++i)
			buffer.PushBack(i);
		EXPECT_EQ(buffer.GetSize(), 3);
		buffer.Resize(6);
		EXPECT_EQ(buffer.GetCapacity(), 6);
		for (int i = 0; i < 3; ++i)
		{
			auto num = buffer.PopFront();
			EXPECT_EQ(num, i);
		}
		EXPECT_EQ(buffer.GetSize(), 0);
		for (int i = 0; i < 6; ++i)
			buffer.PushFront(i);
		EXPECT_EQ(buffer.GetSize(), 6);
		EXPECT_EQ(buffer.GetCapacity(), 6);
		buffer.PushBack(6);
		EXPECT_EQ(buffer.GetCapacity(), uint32_t(6 * 1.5));
		EXPECT_EQ(buffer.GetSize(), 7);
		buffer.Resize(7U);
		EXPECT_EQ(buffer.GetCapacity(), 7U);
		auto num = buffer.PopBack();
		EXPECT_EQ(num, 6);
		EXPECT_EQ(buffer.GetSize(), 6);
		for (int i = 0; i < 6; ++i)
		{
			auto num = buffer.PopBack();
			EXPECT_EQ(num, i);
		}
		EXPECT_EQ(buffer.GetSize(), 0);
		EXPECT_EQ(buffer.GetCapacity(), 7U);
	}

	TEST(CircularBufferTests, CircularBufferComplexStructTest)
	{
		using namespace ECS;
		CircularBuffer<ComplexStruct> buffer(3);

		EXPECT_EQ(buffer.GetCapacity(), 3);
		EXPECT_EQ(buffer.GetSize(), 0);
		EXPECT_EQ(buffer.IsEmpty(), true);

		for (int i = 0; i < 5; ++i)
			buffer.PushBack(6, new char[6] { 'h', 'e', 'l', 'l', 'o', '\0' });

		EXPECT_EQ(buffer.GetSize(), 5);

		CircularBuffer<ComplexStruct> buffer2 = buffer;
		EXPECT_EQ(buffer2.GetSize(), 5);
		EXPECT_EQ(buffer2.GetCapacity(), buffer.GetCapacity());
		for (int i = 0; i < 2; ++i)
		{
			auto complexStruct = buffer2.PopFront();
			EXPECT_EQ(complexStruct.num, 6);
			EXPECT_STRCASEEQ("hello", complexStruct.characters);
		}
		EXPECT_EQ(buffer2.GetSize(), 3);
		for (int i = 0; i < 2; ++i)
			buffer2.PushBack(ComplexStruct(6, new char[6] { 'w', 'o', 'r', 'l', 'd', '\0' }));

		CircularBuffer<ComplexStruct> buffer3 = buffer2;
		EXPECT_EQ(buffer3.GetSize(), 5);
		EXPECT_EQ(buffer3.GetCapacity(), buffer2.GetCapacity());
		for (int i = 0; i < 5; ++i)
		{
			auto complexStruct = buffer3.PopFront();
			EXPECT_EQ(complexStruct.num, 6);
			if (i < 3)
			{
				EXPECT_STRCASEEQ("hello", complexStruct.characters);
				continue;
			}
			EXPECT_STRCASEEQ("world", complexStruct.characters);
		}

		for (auto& complexStruct : buffer)
		{
			EXPECT_EQ(complexStruct.num, 6);
			EXPECT_STRCASEEQ("hello", complexStruct.characters);
			buffer.PopFront();
		}
		EXPECT_EQ(buffer.GetSize(), 0);
		EXPECT_EQ(buffer2.GetSize(), 5);
		EXPECT_EQ(buffer3.GetSize(), 0);
		EXPECT_EQ(buffer2.GetCapacity(), buffer.GetCapacity());
		EXPECT_EQ(buffer3.GetCapacity(), buffer2.GetCapacity());
	}

#if CIRCULAR_BUFFER_STRESS_TEST
	TEST(CircularBufferTests, CircularBufferStressResize)
	{
		using namespace ECS;
		CircularBuffer<uint8_t> buffer(uint32_t(std::numeric_limits<uint32_t>::max() / 2));
		EXPECT_EQ(buffer.GetCapacity(), uint32_t(std::numeric_limits<uint32_t>::max() / 2));
		for (uint32_t i = 0; i < uint32_t(std::numeric_limits<uint32_t>::max() / 1.5); ++i)
			buffer.PushBack(uint8_t(i) % std::numeric_limits<uint8_t>::max());
		EXPECT_EQ(buffer.GetSize(), uint32_t(std::numeric_limits<uint32_t>::max() / 1.5));
		EXPECT_EQ(buffer.GetCapacity(), uint32_t((std::numeric_limits<uint32_t>::max() / 2) * 1.5));
	}
#endif // CIRCULAR_BUFFER_STRESS_TEST


	////////////////////////////////////////////////////////////////////////////////////////
	// ComponentStorage Tests //////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////

	// Unit test to check if all the functions in the ComponentList class work as intended
	TEST(ComponentStorageTest, ComponentStorageGetTest)
	{
		using namespace ECS;
		ComponentList<Transform> transformList;
		ComponentList<A> aList;
		ComponentList<B> bList;
		Transform transform;
		A a;
		B b;
		EntityID entity0 = 0;
		EntityID entity1 = 1;
		EntityID entity2 = 2;
		transformList.Add(entity0, transform);
		aList.Add(entity1, a);
		bList.Add(entity2, b);
		EXPECT_EQ(transformList.GetComponent(entity0), transform);
		EXPECT_EQ(aList.GetComponent(entity1), a);
		EXPECT_EQ(bList.GetComponent(entity2), b);
	}

	TEST(ComponentStorageTest, ComponentStorageHasTest)
	{
		using namespace ECS;
		ComponentList<Transform> transformList;
		ComponentList<A> aList;
		ComponentList<B> bList;
		Transform transform;
		A a;
		B b;
		EntityID entity0 = 0;
		EntityID entity1 = 1;
		EntityID entity2 = 2;
		transformList.Add(entity0, transform);
		aList.Add(entity1, a);
		bList.Add(entity2, b);
		EXPECT_TRUE(transformList.HasComponent(entity0));
		EXPECT_TRUE(aList.HasComponent(entity1));
		EXPECT_TRUE(bList.HasComponent(entity2));
	}

	TEST(ComponentStorageTest, ComponentStorageRemoveTest)
	{
		using namespace ECS;
		ComponentList<Transform> transformList;
		ComponentList<A> aList;
		ComponentList<B> bList;
		Transform transform;
		A a;
		B b;
		EntityID entity0 = 0;
		EntityID entity1 = 1;
		EntityID entity2 = 2;
		transformList.Add(entity0, transform);
		aList.Add(entity1, a);
		bList.Add(entity2, b);
		EXPECT_TRUE(transformList.HasComponent(entity0));
		EXPECT_TRUE(aList.HasComponent(entity1));
		EXPECT_TRUE(bList.HasComponent(entity2));
		transformList.Remove(entity0);
		aList.Remove(entity1);
		bList.Remove(entity2);
		EXPECT_FALSE(transformList.HasComponent(entity0));
		EXPECT_FALSE(aList.HasComponent(entity1));
		EXPECT_FALSE(bList.HasComponent(entity2));
	}

	TEST(ComponentStorageTest, ComponentStorageEmplaceTest)
	{
		using namespace ECS;
		ComponentList<Transform> transformList;
		ComponentList<A> aList;
		ComponentList<B> bList;
		EntityID entity0 = 0;
		EntityID entity1 = 1;

		Transform t = transformList.Emplace(entity0);
		A a = aList.Emplace(entity0);
		B b = bList.Emplace(entity0);

		EXPECT_TRUE(transformList.HasComponent(entity0));
		EXPECT_TRUE(aList.HasComponent(entity0));
		EXPECT_TRUE(bList.HasComponent(entity0));
		EXPECT_EQ(transformList.GetComponent(entity0), t);
		EXPECT_EQ(aList.GetComponent(entity0), a);
		EXPECT_EQ(bList.GetComponent(entity0), b);

		t = transformList.Emplace(entity1, Transform({ 0,0,0 }, { 0, 0, 0 }, { 1, 1, 1 }));
		a = aList.Emplace(entity1, 0);
		b = bList.Emplace(entity1, B());

		EXPECT_EQ(transformList.GetComponent(entity0), t);
		EXPECT_EQ(aList.GetComponent(entity0), a);
		EXPECT_EQ(bList.GetComponent(entity0), b);
		EXPECT_EQ(transformList.Size(), 2);
		EXPECT_EQ(aList.Size(), 2);
		EXPECT_EQ(bList.Size(), 2);
	}

	TEST(ComponentStorageTest, ComponentStorageReplaceOrEmplaceTest)
	{
		
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// EntityRegistry Tests ////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////

	TEST(EntityRegistryTest, CreateEntity)
	{
		using namespace ECS;
		EntityRegistry registry;
		registry.CreateComponentStorage<Transform, A, B>();

		EntityID entity0 = registry.CreateEntity();
		EntityID entity1 = registry.CreateEntity();
		EntityID entity2 = registry.CreateEntity();

		EXPECT_EQ(entity0, 0);
		EXPECT_EQ(entity1, 1);
		EXPECT_EQ(entity2, 2);
	}

	// Unit test to check if all the functions in the EntityRegistry class work as intended
	TEST(EntityRegistryTest, GetComponent)
	{
		using namespace ECS;
		EntityRegistry registry;
		registry.CreateComponentStorage<Transform, A, B>();
		Transform transform;
		A a;
		B b;
		EntityID entity0 = registry.CreateEntity();
		EntityID entity1 = registry.CreateEntity();
		EntityID entity2 = registry.CreateEntity();
		registry.AddComponent(entity0, transform);
		registry.AddComponent(entity1, a);
		registry.AddComponent(entity2, b);
		EXPECT_EQ(registry.GetComponent<Transform>(entity0), transform);
		EXPECT_EQ(registry.GetComponent<A>(entity1), a);
		EXPECT_EQ(registry.GetComponent<B>(entity2), b);
	}

	TEST(EntityRegistryTest, HasComponent)
	{
		using namespace ECS;
		EntityRegistry registry;
		registry.CreateComponentStorage<Transform, A, B>();
		Transform transform;
		A a;
		B b;
		EntityID entity0 = registry.CreateEntity();
		EntityID entity1 = registry.CreateEntity();
		EntityID entity2 = registry.CreateEntity();
		registry.AddComponent(entity0, transform);
		registry.AddComponent(entity1, a);
		registry.AddComponent(entity2, b);
		EXPECT_TRUE(registry.HasComponent<Transform>(entity0));
		EXPECT_TRUE(registry.HasComponent<A>(entity1));
		EXPECT_TRUE(registry.HasComponent<B>(entity2));
	}

	TEST(EntityRegistryTest, DeleteComponent)
	{
		using namespace ECS;
		EntityRegistry registry;
		registry.CreateComponentStorage<Transform, A, B>();
		Transform transform;
		A a;
		B b;
		EntityID entity0 = registry.CreateEntity();
		EntityID entity1 = registry.CreateEntity();
		EntityID entity2 = registry.CreateEntity();
		registry.AddComponent(entity0, transform);
		registry.AddComponent(entity1, a);
		registry.AddComponent(entity2, b);
		registry.AddComponent(entity0, a);
		registry.AddComponent(entity1, b);
		registry.AddComponent(entity2, transform);
		EXPECT_TRUE(registry.HasComponent<Transform>(entity0));
		EXPECT_TRUE(registry.HasComponent<A>(entity1));
		EXPECT_TRUE(registry.HasComponent<B>(entity2));
		registry.DeleteComponent<Transform>(entity0);
		registry.DeleteComponent<A>(entity1);
		registry.DeleteComponent<B>(entity2);
		EXPECT_FALSE(registry.HasComponent<Transform>(entity0));
		EXPECT_FALSE(registry.HasComponent<A>(entity1));
		EXPECT_FALSE(registry.HasComponent<B>(entity2));
	}

	TEST(EntityRegistryTest, DeleteEntity)
	{
		using namespace ECS;
		EntityRegistry registry;
		registry.CreateComponentStorage<Transform, A, B>();
		Transform transform;
		A a;
		B b;
		EntityID entity0 = registry.CreateEntity();
		registry.AddComponent(entity0, transform);
		registry.AddComponent(entity0, b);
		EXPECT_TRUE(registry.HasComponent<Transform>(entity0));
		EXPECT_TRUE(registry.HasComponent<B>(entity0));
		registry.DeleteEntity(entity0);
		EXPECT_FALSE(registry.HasComponent<Transform>(entity0));
		EXPECT_FALSE(registry.HasComponent<B>(entity0));
		registry.DeleteEntity(entity0);
	}

}