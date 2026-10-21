#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout
OrbitTextureAudioProcessor::createParameters()
{
    using P = juce::AudioParameterFloat;
    using R = juce::NormalisableRange<float>;
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    p.add(std::make_unique<P>("ORBIT", "ORBIT", R(0.0f, 1.0f), 0.15f));
    p.add(std::make_unique<P>("SPREAD", "SPREAD", R(0.0f, 1.0f), 0.3f));
    p.add(std::make_unique<P>("COLOR", "COLOR", R(0.0f, 1.0f), 0.5f));
    p.add(std::make_unique<P>("NOVA", "NOVA", R(0.0f, 1.0f), 0.2f));
    p.add(std::make_unique<P>("TAIL", "TAIL", R(0.0f, 1.0f), 0.4f));
    return p;
}

OrbitTextureAudioProcessor::OrbitTextureAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameters())
{
}

void OrbitTextureAudioProcessor::prepareToPlay(double sampleRate, int)
{
    sr = juce::jmax(1.0, sampleRate);
    auto init = [this](juce::LinearSmoothedValue<float>& s, const char* id) {
        s.reset(sr, 0.025);
        s.setCurrentAndTargetValue(*parameters.getRawParameterValue(id));
    };
    init(orbit, "ORBIT"); init(spread, "SPREAD"); init(color, "COLOR");
    init(nova, "NOVA"); init(tail, "TAIL");

    juce::dsp::ProcessSpec spec{sr, 2048u, 1u};
    filterL.prepare(spec); filterR.prepare(spec);
    filterL.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    filterR.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    filterL.setResonance(0.707f); filterR.setResonance(0.707f);
    juce::dsp::ProcessSpec stereoSpec{sr, 2048u, 2u};
    reverb.prepare(stereoSpec);
    juce::dsp::Reverb::Parameters rp;
    rp.roomSize = 0.75f; rp.damping = 0.45f; rp.width = 1.0f;
    rp.wetLevel = 0.2f; rp.dryLevel = 1.0f;
    reverb.setParameters(rp);
    for (auto& v : voices) v = Voice{};
}

void OrbitTextureAudioProcessor::noteOn(int note, float velocity)
{
    Voice* v = nullptr;
    for (auto& x : voices) if (!x.active) { v = &x; break; }
    if (!v) v = &voices[0];
    *v = Voice{};
    v->active = true; v->note = note; v->velocity = velocity;
    // Orbit randomises the loop's phase on each trigger.
    v->loopPhase = juce::Random::getSystemRandom().nextDouble()
                   * juce::jlimit(0.0, 1.0, (double)*parameters.getRawParameterValue("ORBIT"));
}

void OrbitTextureAudioProcessor::noteOff(int note)
{
    const float t = tail.getCurrentValue();
    const float seconds = 0.08f + 7.0f * t;
    for (auto& v : voices) if (v.active && v.note == note && !v.releasing) {
        v.releasing = true;
        v.releaseStep = 1.0f / (float)juce::jmax(1.0, seconds * sr);
    }
}

void OrbitTextureAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                               juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    auto setTarget = [this](juce::LinearSmoothedValue<float>& s, const char* id) {
        s.setTargetValue(*parameters.getRawParameterValue(id));
    };
    setTarget(orbit, "ORBIT"); setTarget(spread, "SPREAD"); setTarget(color, "COLOR");
    setTarget(nova, "NOVA"); setTarget(tail, "TAIL");

    if (auto* head = getPlayHead()) {
        if (auto pos = head->getPosition()) {
            if (auto hostBpm = pos->getBpm())
                if (std::isfinite(*hostBpm) && *hostBpm >= 20.0 && *hostBpm <= 300.0)
                    bpm = (float)*hostBpm;
        }
    }

    for (const auto md : midi) {
        auto m = md.getMessage();
        if (m.isNoteOn()) noteOn(m.getNoteNumber(), m.getFloatVelocity());
        else if (m.isNoteOff()) noteOff(m.getNoteNumber());
        else if (m.isAllNotesOff() || m.isAllSoundOff())
            for (auto& v : voices) v.active = false;
    }

    const int n = buffer.getNumSamples();
    const int chs = buffer.getNumChannels();
    if (chs < 1) return;

    for (int i = 0; i < n; ++i) {
        const float o = orbit.getNextValue();
        const float w = spread.getNextValue();
        const float c = color.getNextValue();
        const float rv = nova.getNextValue();
        const float tv = tail.getNextValue();

        float l = 0.0f, r = 0.0f;
        for (auto& v : voices) {
            if (!v.active) continue;
            const double f = 440.0 * std::pow(2.0, (v.note - 69) / 12.0);
            const double inc = juce::MathConstants<double>::twoPi * f / sr;
            // One-shot-like harmonic engine: fundamental plus a soft upper partial.
            const float shot = (float)(std::sin(v.phase) + 0.22 * std::sin(2.0 * v.phase));
            v.phase += inc;
            if (v.phase >= juce::MathConstants<double>::twoPi)
                v.phase -= juce::MathConstants<double>::twoPi;

            // Loop-like engine: a four-beat repeating rhythmic texture.
            const double loopHz = 2.0 * bpm / 60.0;
            const double loopInc = juce::MathConstants<double>::twoPi * loopHz / sr;
            const float pulse = (float)(std::sin(v.loopPhase) > (0.25 - 0.45 * o) ? 1.0 : -0.25);
            const float loop = pulse * (float)(0.65 + 0.35 * std::sin(v.loopPhase * 0.5));
            v.loopPhase += loopInc * (1.0 + o * 0.08);
            if (v.loopPhase >= juce::MathConstants<double>::twoPi)
                v.loopPhase -= juce::MathConstants<double>::twoPi;

            if (!v.releasing) v.env = juce::jmin(1.0f, v.env + 1.0f / (float)(0.015 * sr));
            else {
                v.env -= v.releaseStep;
                if (v.env <= 0.0f) { v.active = false; continue; }
            }
            const float sample = (shot * 0.75f + loop * 0.12f) * v.env * v.velocity;
            const float pan = juce::jlimit(-0.9f, 0.9f, (float)std::sin(v.loopPhase) * w);
            l += sample * std::sqrt(0.5f * (1.0f - pan));
            r += sample * std::sqrt(0.5f * (1.0f + pan));
            juce::ignoreUnused(tv);
        }

        // Macro COLOR sweeps from warm low-pass to bright high-pass.
        const bool low = c < 0.5f;
        const float amount = low ? (0.5f - c) * 2.0f : (c - 0.5f) * 2.0f;
        const float cutoff = low ? 20000.0f - amount * 18500.0f : 200.0f + amount * 8000.0f;
        const auto type = low ? juce::dsp::StateVariableTPTFilterType::lowpass
                              : juce::dsp::StateVariableTPTFilterType::highpass;
        filterL.setType(type); filterR.setType(type);
        filterL.setCutoffFrequency(cutoff); filterR.setCutoffFrequency(cutoff);
        l = juce::jmap(amount, l, filterL.processSample(0, l));
        r = juce::jmap(amount, r, filterR.processSample(0, r));

        // SPREAD expands side information while preserving the mono centre.
        const float mid = 0.5f * (l + r);
        const float side = 0.5f * (l - r) * (1.0f + w);
        buffer.setSample(0, i, (mid + side) * 0.18f);
        if (chs > 1) buffer.setSample(1, i, (mid - side) * 0.18f);

        auto rp = reverb.getParameters();
        rp.wetLevel = rv * 0.55f;
        rp.dryLevel = 1.0f;
        reverb.setParameters(rp);
    }

    if (chs > 1) {
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> ctx(block);
        reverb.process(ctx);
    }
}

void OrbitTextureAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto xml = parameters.copyState().createXml();
    copyXmlToBinary(*xml, dest);
}

void OrbitTextureAudioProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* OrbitTextureAudioProcessor::createEditor()
{
    return new OrbitTextureAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OrbitTextureAudioProcessor();
}
