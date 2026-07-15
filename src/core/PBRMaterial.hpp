#ifndef __PBRMaterial_hpp__
#define __PBRMaterial_hpp__

#include "Material.hpp"

struct alignas(16) PBRMaterialProperties {
    // Block 1 (16 bytes)
    float    albedo_r, albedo_g, albedo_b;
    float    metallic;          // Packed directly after Albedo (3 floats + 1 float = 16 bytes)

    // Block 2 (16 bytes)
    float    roughness;
    float    ior;
    float    transmission;
    uint32_t albedo_map_id;     // Mixed float/uint block (4 components = 16 bytes)

    // Block 3 (16 bytes)
    float    emissive_r, emissive_g, emissive_b;
    float    emissive_intensity;// Packed together (3 floats + 1 float = 16 bytes)

    // Block 4 (16 bytes)
    uint32_t metallic_map_id;
    uint32_t roughness_map_id;
    uint32_t normal_map_id;
    uint32_t emissive_map_id;   // 4 texture IDs (16 bytes)

    // Block 5 (16 bytes)
    uint32_t ao_map_id;
    uint32_t is_transparent;
    uint32_t has_emissive;
    uint32_t padding;           // Explicit padding to keep the whole struct 16-byte aligned
};

class PBRMaterial : public Material
{
public:
	PBRMaterial(const std::string& name);
	virtual ~PBRMaterial() = default;

    // --- GETTERS & SETTERS ---

    // Base Properties
    const void getAlbedoColor(float& r, float& g, float& b) const {
        r = m_data.albedo_r;
        g = m_data.albedo_g;
        b = m_data.albedo_b;
    }

    void setAlbedoColor(const float color[3]) {
        m_data.albedo_r = color[0];
        m_data.albedo_g = color[1];
        m_data.albedo_b = color[2];
        m_is_dirty = true;
    }

    float getMetallic() const { return m_data.metallic; }
    void setMetallic(float metallic) { m_data.metallic = metallic; m_is_dirty = true; }

    float GetRoughness() const { return m_data.roughness; }
    void SetRoughness(float roughness) { m_data.roughness = roughness; m_is_dirty = true; }

    // Advanced Properties
    float getIOR() const { return m_data.ior; }
    void setIOR(float ior) { m_data.ior = ior; m_is_dirty = true; }

    float getTransmission() const { return m_data.transmission; }
    void setTransmission(float transmission) { m_data.transmission = transmission; m_is_dirty = true; }

    // Emissive Properties
    const void getEmissiveColor(float& r, float& g, float &b) const { 
        r = m_data.emissive_r;
        g = m_data.emissive_g;
        b = m_data.emissive_b;
    }
    void setEmissiveColor(const float color[3]) { 
        m_data.emissive_r = color[0];
        m_data.emissive_g = color[1];
        m_data.emissive_b = color[2];
        m_is_dirty = true; 
    }

    float getEmissiveIntensity() const { return m_data.emissive_intensity; }
    void setEmissiveIntensity(float intensity) { m_data.emissive_intensity = intensity; m_is_dirty = true; }

    // Texture Maps
    uint32_t getAlbedoMapId() const { return m_data.albedo_map_id; }
    void setAlbedoMapId(uint32_t id) { m_data.albedo_map_id = id; m_is_dirty = true; }

    uint32_t getMetallicMapId() const { return m_data.metallic_map_id; }
    void setMetallicMapId(uint32_t id) { m_data.metallic_map_id = id; m_is_dirty = true; }

    uint32_t getRoughnessMapId() const { return m_data.roughness_map_id; }
    void setRoughnessMapId(uint32_t id) { m_data.roughness_map_id = id; m_is_dirty = true; }

    uint32_t getNormalMapId() const { return m_data.normal_map_id; }
    void setNormalMapId(uint32_t id) { m_data.normal_map_id = id; m_is_dirty = true; }

    uint32_t getEmissiveMapId() const { return m_data.emissive_map_id; }
    void setEmissiveMapId(uint32_t id) { m_data.emissive_map_id = id; m_is_dirty = true; }

    uint32_t getAOMapId() const { return m_data.ao_map_id; }
    void setAOMapId(uint32_t id) { m_data.ao_map_id = id; m_is_dirty = true; }

    // Optimization Flags
    bool isTransparent() const { return m_data.is_transparent != 0; }
    void setTransparent(bool transparent) { m_data.is_transparent = transparent ? 1 : 0; m_is_dirty = true; }

    bool hasEmissiveFlag() const { return m_data.has_emissive != 0; }
    void setHasEmissiveFlag(bool hasEmissive) { m_data.has_emissive = hasEmissive ? 1 : 0; m_is_dirty = true; }

    // --- ENGINE UTILITIES ---

    // Quick check to see if the ray tracer should treat this as an active light source
    bool isActivelyEmissive() const {
        return m_data.has_emissive != 0 && m_data.emissive_intensity > 0.0f;
    }

    // GPU Synchronization Helpers
    void clearDirty() { m_is_dirty = false; }
    bool isDirty() const { return m_is_dirty; }

	void set_properties(const PBRMaterialProperties& properties) {
		m_data = properties;
		m_is_dirty = true;
	}
private:
    PBRMaterialProperties m_data;

    // Synchronization state (not sent to GPU)
    bool m_is_dirty{ true };
};

#endif
